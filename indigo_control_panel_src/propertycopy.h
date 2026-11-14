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


#ifndef PROPERTYCOPY_H
#define PROPERTYCOPY_H

#include <indigo/indigo_bus.h>

//  Deep copy a property received from the bus. The copy is sized for
//  property->count items, keeps its own allocated_count and owns its own
//  long text buffers. Release it with indigo_release_property().
indigo_property *copy_property(indigo_property *property);

//  Copy the items of src into dst, matching them by name. Items of src that
//  dst does not define are ignored (dst's item array is never resized, as
//  widgets hold pointers into it). For text properties dst takes ownership
//  of src's long text buffers and src's pointers are cleared, so releasing
//  src afterwards is safe and nothing leaks. dst->state is not touched.
void merge_property_items(indigo_property *dst, indigo_property *src);

#endif // PROPERTYCOPY_H
