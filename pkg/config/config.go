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
	"reflect"
	"strconv"
	"sync"
)

type ConfigKV[T int | bool | string] struct {
	store sync.Map
}

func NewConfigKV[T int | bool | string]() *ConfigKV[T] {
	return &ConfigKV[T]{}
}

func (c *ConfigKV[T]) Set(k string, v T) {
	c.store.Store(k, v)
}
func (c *ConfigKV[T]) Get(k string) (T, bool) {
	v, ok := c.store.Load(k)

	var t T
	if ok {
		return v.(T), ok
	} else {
		return t, ok
	}
}
func (c *ConfigKV[T]) GetWithDefault(k string, defaultValue T) T {
	v, ok := c.store.Load(k)
	if ok {
		return v.(T)
	} else {
		return defaultValue
	}
}

func (c *ConfigKV[T]) List() []string {

	l := make([]string, 0)
	c.store.Range(func(k, v any) bool {
		var cons string
		switch reflect.ValueOf(v).Kind() {
		case reflect.Int:
			cons = strconv.Itoa(v.(int))
		case reflect.String:
			cons = v.(string)
		case reflect.Bool:
			cons = strconv.FormatBool(v.(bool))
		default:
			cons = "type unknown"
		}

		l = append(l, k.(string)+"="+cons)
		return true
	})
	return l
}

var (
	IntKV    *ConfigKV[int]
	StringKV *ConfigKV[string]
	BoolKV   *ConfigKV[bool]
)

func init() {
	IntKV = NewConfigKV[int]()
	StringKV = NewConfigKV[string]()
	BoolKV = NewConfigKV[bool]()
}
