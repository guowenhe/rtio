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

package timekv

import (
	"testing"
	"time"

	"gotest.tools/assert"
)

func TestSetAndGet(t *testing.T) {

	kv := NewTimeKV(time.Second * 5)
	kv.Set(99, &Value{internelID: 9999})
	kv.Set(100, &Value{internelID: 10000})
	kv.Set(99, &Value{internelID: 99991})
	kv.Set(101, &Value{internelID: 10100})

	v, ok := kv.Get(99)
	assert.Equal(t, ok, true)
	assert.Equal(t, v.internelID, 99991)
	t.Logf("v.internelID=%v", v.internelID)

	v, ok = kv.Get(100)
	assert.Equal(t, ok, true)
	assert.Equal(t, v.internelID, 10000)
	t.Logf("v.internelID=%v", v.internelID)

	kv.Del(99)
	_, ok = kv.Get(99)
	assert.Equal(t, ok, false)

}

func TestDelExpireKeys(t *testing.T) {

	kv := NewTimeKV(time.Second * 3)
	kv.Set(99, &Value{internelID: 9999})
	kv.Set(100, &Value{internelID: 10000})
	time.Sleep(time.Second * 1) //after 1 s
	kv.Set(102, &Value{internelID: 10200})
	kv.Set(103, &Value{internelID: 10300})

	time.Sleep(time.Second * 2)
	kv.DelExpireKeys()

	_, ok := kv.Get(99)
	assert.Equal(t, ok, false)
	_, ok = kv.Get(100)
	assert.Equal(t, ok, false)

	v, ok := kv.Get(102)
	assert.Equal(t, ok, true)
	assert.Equal(t, v.internelID, 10200)
	t.Logf("v.internelID=%v", v.internelID)

	t.Logf("-------------")
	kv.RangeKV(func(k Key, v *Value) bool {
		t.Logf("k=%v,v=%v\n", k, v.internelID)
		return true
	})

}
