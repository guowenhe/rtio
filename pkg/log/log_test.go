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

package log

import (
	"testing"
)

func TestLog(t *testing.T) {
	Debug().Msg("hello world 5")
	Info().Str("name", "hello world")
	arr := []int{1, 2, 4}
	Debug().Int("id", 324).Ints("ids", arr).Send()
}
