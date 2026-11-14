// Copyright (c) 2026 Rumen G.Bogdanovski
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//  Tests for propertycopy.cpp. The test binary is built with
//  AddressSanitizer, so use-after-free, out-of-bounds access and memory
//  leaks fail the run even where no explicit QCOMPARE would notice.

#include <QtTest>
#include <string>
#include <indigo/indigo_bus.h>
#include "propertycopy.h"

static std::string long_text(char c, size_t len = 3 * INDIGO_VALUE_SIZE) {
	std::string s;
	s.reserve(len);
	for (size_t i = 0; i < len; i++) {
		s += (i % 64 == 63) ? '\n' : (char)(c + i % 10);
	}
	return s;
}

static indigo_property *make_text(int count, const char *names[], const char *values[], int allocated = 0) {
	//  allocated > count mimics properties coming from the XML parser,
	//  which preallocates INDIGO_PREALLOCATED_COUNT items.
	indigo_property *p = indigo_init_text_property(nullptr, "Dev", "PROP", "Group", "Label", INDIGO_OK_STATE, INDIGO_RW_PERM, allocated > count ? allocated : count);
	p->count = count;
	for (int i = 0; i < count; i++) {
		indigo_init_text_item_raw(&p->items[i], names[i], names[i], values[i]);
	}
	return p;
}

class TestPropertyCopy : public QObject {
	Q_OBJECT

private slots:
	void copy_keeps_own_allocated_count();
	void copy_deep_copies_long_text();
	void copy_number_property();
	void merge_long_text_survives_release_of_source();
	void merge_long_replaced_by_short();
	void merge_short_replaced_by_long();
	void merge_ignores_items_not_in_destination();
	void merge_matches_items_by_name();
	void merge_subset_leaves_other_items_unchanged();
	void merge_number_property();
	void repeated_updates_do_not_leak();
};

void TestPropertyCopy::copy_keeps_own_allocated_count() {
	const char *names[] = { "A", "B" };
	std::string big = long_text('a');
	const char *values[] = { "short", big.c_str() };
	indigo_property *src = make_text(2, names, values, INDIGO_PREALLOCATED_COUNT);
	QCOMPARE(src->allocated_count, INDIGO_PREALLOCATED_COUNT);

	indigo_property *copy = copy_property(src);
	QVERIFY(copy != nullptr);
	QCOMPARE(copy->count, 2);
	QCOMPARE(copy->allocated_count, 2);
	//  Without the fix this release walks 512 items of a 2 item buffer.
	indigo_release_property(copy);
	indigo_release_property(src);
}

void TestPropertyCopy::copy_deep_copies_long_text() {
	const char *names[] = { "SCRIPT" };
	std::string big = long_text('a');
	const char *values[] = { big.c_str() };
	indigo_property *src = make_text(1, names, values);
	QVERIFY(src->items[0].text.long_value != nullptr);

	indigo_property *copy = copy_property(src);
	QVERIFY(copy->items[0].text.long_value != nullptr);
	QVERIFY(copy->items[0].text.long_value != src->items[0].text.long_value);
	indigo_release_property(src);

	QCOMPARE(QString(indigo_get_text_item_value(&copy->items[0])), QString::fromStdString(big));
	indigo_release_property(copy);
}

void TestPropertyCopy::copy_number_property() {
	indigo_property *src = indigo_init_number_property(nullptr, "Dev", "NUM", "Group", "Label", INDIGO_OK_STATE, INDIGO_RW_PERM, INDIGO_PREALLOCATED_COUNT);
	src->count = 2;
	indigo_init_number_item(&src->items[0], "X", "X", 0, 100, 1, 42);
	indigo_init_number_item(&src->items[1], "Y", "Y", 0, 100, 1, 7);

	indigo_property *copy = copy_property(src);
	QCOMPARE(copy->allocated_count, 2);
	QCOMPARE(copy->items[0].number.value, 42.0);
	QCOMPARE(copy->items[1].number.value, 7.0);
	indigo_release_property(copy);
	indigo_release_property(src);
}

void TestPropertyCopy::merge_long_text_survives_release_of_source() {
	const char *names[] = { "SCRIPT" };
	std::string old_text = long_text('a');
	std::string new_text = long_text('k', 5 * INDIGO_VALUE_SIZE);
	const char *old_values[] = { old_text.c_str() };
	const char *new_values[] = { new_text.c_str() };
	indigo_property *cached = make_text(1, names, old_values);
	indigo_property *update = make_text(1, names, new_values);

	merge_property_items(cached, update);
	QVERIFY(update->items[0].text.long_value == nullptr);
	//  This is what PropertyModel::update_property does next. Before the
	//  fix it freed the buffer the cached item still pointed to.
	indigo_release_property(update);

	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString::fromStdString(new_text));
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_long_replaced_by_short() {
	const char *names[] = { "SCRIPT" };
	std::string old_text = long_text('a');
	const char *old_values[] = { old_text.c_str() };
	const char *new_values[] = { "print('hi')" };
	indigo_property *cached = make_text(1, names, old_values);
	indigo_property *update = make_text(1, names, new_values);

	merge_property_items(cached, update);
	indigo_release_property(update);

	QVERIFY(cached->items[0].text.long_value == nullptr);
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString("print('hi')"));
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_short_replaced_by_long() {
	const char *names[] = { "SCRIPT" };
	std::string new_text = long_text('k');
	const char *old_values[] = { "" };
	const char *new_values[] = { new_text.c_str() };
	indigo_property *cached = make_text(1, names, old_values);
	indigo_property *update = make_text(1, names, new_values);

	merge_property_items(cached, update);
	indigo_release_property(update);

	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString::fromStdString(new_text));
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_ignores_items_not_in_destination() {
	const char *cached_names[] = { "A" };
	const char *cached_values[] = { "a" };
	const char *update_names[] = { "A", "B", "C" };
	std::string big = long_text('b');
	const char *update_values[] = { "a2", big.c_str(), "c" };
	indigo_property *cached = make_text(1, cached_names, cached_values);
	indigo_property *update = make_text(3, update_names, update_values);

	//  Before the fix this wrote 3 items into a 1 item buffer.
	merge_property_items(cached, update);
	QCOMPARE(cached->count, 1);
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString("a2"));
	//  "B"'s long buffer was not taken over, so the update must still own it.
	QVERIFY(update->items[1].text.long_value != nullptr);
	indigo_release_property(update);
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_matches_items_by_name() {
	const char *cached_names[] = { "A", "B" };
	const char *cached_values[] = { "a", "b" };
	const char *update_names[] = { "B", "A" };
	std::string big = long_text('x');
	const char *update_values[] = { big.c_str(), "a2" };
	indigo_property *cached = make_text(2, cached_names, cached_values);
	indigo_property *update = make_text(2, update_names, update_values);

	merge_property_items(cached, update);
	indigo_release_property(update);

	QCOMPARE(QString(cached->items[0].name), QString("A"));
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString("a2"));
	QCOMPARE(QString(cached->items[1].name), QString("B"));
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[1])), QString::fromStdString(big));
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_subset_leaves_other_items_unchanged() {
	const char *cached_names[] = { "A", "B" };
	std::string big = long_text('a');
	const char *cached_values[] = { big.c_str(), "b" };
	const char *update_names[] = { "B" };
	const char *update_values[] = { "b2" };
	indigo_property *cached = make_text(2, cached_names, cached_values);
	indigo_property *update = make_text(1, update_names, update_values);

	merge_property_items(cached, update);
	indigo_release_property(update);

	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString::fromStdString(big));
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[1])), QString("b2"));
	indigo_release_property(cached);
}

void TestPropertyCopy::merge_number_property() {
	indigo_property *cached = indigo_init_number_property(nullptr, "Dev", "NUM", "Group", "Label", INDIGO_OK_STATE, INDIGO_RW_PERM, 2);
	indigo_init_number_item(&cached->items[0], "X", "X", 0, 100, 1, 1);
	indigo_init_number_item(&cached->items[1], "Y", "Y", 0, 100, 1, 2);
	indigo_property *update = indigo_init_number_property(nullptr, "Dev", "NUM", "Group", "Label", INDIGO_OK_STATE, INDIGO_RW_PERM, 3);
	indigo_init_number_item(&update->items[0], "Y", "Y", 0, 100, 1, 20);
	indigo_init_number_item(&update->items[1], "Z", "Z", 0, 100, 1, 30);
	indigo_init_number_item(&update->items[2], "X", "X", 0, 100, 1, 10);

	merge_property_items(cached, update);
	indigo_release_property(update);

	QCOMPARE(cached->items[0].number.value, 10.0);
	QCOMPARE(cached->items[1].number.value, 20.0);
	indigo_release_property(cached);
}

void TestPropertyCopy::repeated_updates_do_not_leak() {
	//  Full client -> model path: every update is deep copied from a
	//  parser-like property, merged into the cache and released. The old
	//  cached buffer must be freed each time (LeakSanitizer checks at exit).
	const char *names[] = { "SCRIPT", "NAME" };
	std::string first = long_text('a');
	const char *values[] = { first.c_str(), "n" };
	indigo_property *bus = make_text(2, names, values, INDIGO_PREALLOCATED_COUNT);
	indigo_property *cached = copy_property(bus);
	indigo_release_property(bus);

	std::string last;
	for (int i = 0; i < 50; i++) {
		last = long_text('a' + i % 20, 2 * INDIGO_VALUE_SIZE + i);
		const char *v[] = { last.c_str(), "n" };
		bus = make_text(2, names, v, INDIGO_PREALLOCATED_COUNT);
		indigo_property *update = copy_property(bus);
		indigo_release_property(bus);
		merge_property_items(cached, update);
		indigo_release_property(update);
	}
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[0])), QString::fromStdString(last));
	QCOMPARE(QString(indigo_get_text_item_value(&cached->items[1])), QString("n"));
	indigo_release_property(cached);
}

QTEST_APPLESS_MAIN(TestPropertyCopy)
#include "tst_propertycopy.moc"
