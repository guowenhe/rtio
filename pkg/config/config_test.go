/*
*
* Copyright 2023 RTIO authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
 */
package config

import (
	"testing"

	"gotest.tools/assert"
)

func TestStringKVSetAndGet(t *testing.T) {

	StringKV.Set("stringaaa", "aaa")
	StringKV.Set("stringbbb", "bbb")

	stringaaa, ok := StringKV.Get("stringaaa")
	t.Logf("stringaaa=%v", stringaaa)
	assert.Equal(t, stringaaa, "aaa")
	assert.Equal(t, ok, true)

	_, ok = StringKV.Get("stringaab")
	assert.Equal(t, ok, false)

	stringbbb, ok := StringKV.Get("stringbbb")
	t.Logf("stringbbb=%v", stringbbb)
	assert.Equal(t, stringbbb, "bbb")
	assert.Equal(t, ok, true)

	stringbbb = StringKV.GetWithDefault("stringbbb", "bbb-default")
	t.Logf("stringbbb=%v", stringbbb)
	assert.Equal(t, stringbbb, "bbb")

	stringbbc := StringKV.GetWithDefault("stringbbc", "bbb-default")
	t.Logf("stringbbb=%v", stringbbc)
	assert.Equal(t, stringbbc, "bbb-default")

	l := StringKV.List()
	t.Logf("l=%v", l)
	assert.Equal(t, len(l), 2)
	assert.Equal(t, l[0], "stringaaa=aaa")
	assert.Equal(t, l[1], "stringbbb=bbb")

}

func TestIntKVSetAndGet(t *testing.T) {

	IntKV.Set("intaaa", 1)
	IntKV.Set("intbbb", 2)

	intaaa, ok := IntKV.Get("intaaa")
	t.Logf("intaaa=%v", intaaa)
	assert.Equal(t, intaaa, 1)
	assert.Equal(t, ok, true)

	_, ok = IntKV.Get("intaab")
	assert.Equal(t, ok, false)

	intbbb, ok := IntKV.Get("intbbb")
	t.Logf("intbbb=%v", intbbb)
	assert.Equal(t, intbbb, 2)
	assert.Equal(t, ok, true)

	intbbb = IntKV.GetWithDefault("intbbb", 3)
	t.Logf("intbbb=%v", intbbb)
	assert.Equal(t, intbbb, 2)

	intbbc := IntKV.GetWithDefault("intbbc", 3)
	t.Logf("intbbb=%v", intbbc)
	assert.Equal(t, intbbc, 3)

	l := IntKV.List()
	t.Logf("l=%v", l)
	assert.Equal(t, len(l), 2)
	assert.Equal(t, l[0], "intaaa=1")
	assert.Equal(t, l[1], "intbbb=2")
}

func TestBoolKVSetAndGet(t *testing.T) {

	BoolKV.Set("boolaaa", true)
	BoolKV.Set("boolbbb", false)

	boolaaa, ok := BoolKV.Get("boolaaa")
	t.Logf("boolaaa=%v", boolaaa)
	assert.Equal(t, boolaaa, true)
	assert.Equal(t, ok, true)

	_, ok = BoolKV.Get("boolaab")
	assert.Equal(t, ok, false)

	boolbbb, ok := BoolKV.Get("boolbbb")
	t.Logf("boolbbb=%v", boolbbb)
	assert.Equal(t, boolbbb, false)
	assert.Equal(t, ok, true)

	boolbbb = BoolKV.GetWithDefault("boolbbb", true)
	t.Logf("boolbbb=%v", boolbbb)
	assert.Equal(t, boolbbb, false)

	boolbbc := BoolKV.GetWithDefault("boolbbc", true)
	t.Logf("boolbbb=%v", boolbbc)
	assert.Equal(t, boolbbc, true)

	l := BoolKV.List()
	t.Logf("l=%v", l)
	assert.Equal(t, len(l), 2)
	assert.Equal(t, l[0], "boolaaa=true")
	assert.Equal(t, l[1], "boolbbb=false")
}
