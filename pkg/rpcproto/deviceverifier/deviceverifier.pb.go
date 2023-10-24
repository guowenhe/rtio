// device verifier

// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.30.0
// 	protoc        v3.6.1
// source: deviceverifier/deviceverifier.proto

package deviceverifier

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type Code int32

const (
	Code_CODE_UNKNOW             Code = 0
	Code_CODE_PASS               Code = 1
	Code_CODE_FAIL               Code = 2
	Code_CODE_DEVICEID_NOT_FOUND Code = 3
)

// Enum value maps for Code.
var (
	Code_name = map[int32]string{
		0: "CODE_UNKNOW",
		1: "CODE_PASS",
		2: "CODE_FAIL",
		3: "CODE_DEVICEID_NOT_FOUND",
	}
	Code_value = map[string]int32{
		"CODE_UNKNOW":             0,
		"CODE_PASS":               1,
		"CODE_FAIL":               2,
		"CODE_DEVICEID_NOT_FOUND": 3,
	}
)

func (x Code) Enum() *Code {
	p := new(Code)
	*p = x
	return p
}

func (x Code) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (Code) Descriptor() protoreflect.EnumDescriptor {
	return file_deviceverifier_deviceverifier_proto_enumTypes[0].Descriptor()
}

func (Code) Type() protoreflect.EnumType {
	return &file_deviceverifier_deviceverifier_proto_enumTypes[0]
}

func (x Code) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use Code.Descriptor instead.
func (Code) EnumDescriptor() ([]byte, []int) {
	return file_deviceverifier_deviceverifier_proto_rawDescGZIP(), []int{0}
}

type VerifyReq struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id           uint32 `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	DeviceId     string `protobuf:"bytes,2,opt,name=device_id,json=deviceId,proto3" json:"device_id,omitempty"`
	DeviceSecret string `protobuf:"bytes,3,opt,name=device_secret,json=deviceSecret,proto3" json:"device_secret,omitempty"`
}

func (x *VerifyReq) Reset() {
	*x = VerifyReq{}
	if protoimpl.UnsafeEnabled {
		mi := &file_deviceverifier_deviceverifier_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *VerifyReq) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*VerifyReq) ProtoMessage() {}

func (x *VerifyReq) ProtoReflect() protoreflect.Message {
	mi := &file_deviceverifier_deviceverifier_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use VerifyReq.ProtoReflect.Descriptor instead.
func (*VerifyReq) Descriptor() ([]byte, []int) {
	return file_deviceverifier_deviceverifier_proto_rawDescGZIP(), []int{0}
}

func (x *VerifyReq) GetId() uint32 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *VerifyReq) GetDeviceId() string {
	if x != nil {
		return x.DeviceId
	}
	return ""
}

func (x *VerifyReq) GetDeviceSecret() string {
	if x != nil {
		return x.DeviceSecret
	}
	return ""
}

type VerifyResp struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Id   uint32 `protobuf:"varint,1,opt,name=id,proto3" json:"id,omitempty"`
	Code Code   `protobuf:"varint,2,opt,name=code,proto3,enum=deviceverifier.Code" json:"code,omitempty"`
}

func (x *VerifyResp) Reset() {
	*x = VerifyResp{}
	if protoimpl.UnsafeEnabled {
		mi := &file_deviceverifier_deviceverifier_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *VerifyResp) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*VerifyResp) ProtoMessage() {}

func (x *VerifyResp) ProtoReflect() protoreflect.Message {
	mi := &file_deviceverifier_deviceverifier_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use VerifyResp.ProtoReflect.Descriptor instead.
func (*VerifyResp) Descriptor() ([]byte, []int) {
	return file_deviceverifier_deviceverifier_proto_rawDescGZIP(), []int{1}
}

func (x *VerifyResp) GetId() uint32 {
	if x != nil {
		return x.Id
	}
	return 0
}

func (x *VerifyResp) GetCode() Code {
	if x != nil {
		return x.Code
	}
	return Code_CODE_UNKNOW
}

var File_deviceverifier_deviceverifier_proto protoreflect.FileDescriptor

var file_deviceverifier_deviceverifier_proto_rawDesc = []byte{
	0x0a, 0x23, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72,
	0x2f, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0e, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x76, 0x65, 0x72,
	0x69, 0x66, 0x69, 0x65, 0x72, 0x22, 0x5d, 0x0a, 0x09, 0x76, 0x65, 0x72, 0x69, 0x66, 0x79, 0x52,
	0x65, 0x71, 0x12, 0x0e, 0x0a, 0x02, 0x69, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x02,
	0x69, 0x64, 0x12, 0x1b, 0x0a, 0x09, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x5f, 0x69, 0x64, 0x18,
	0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x49, 0x64, 0x12,
	0x23, 0x0a, 0x0d, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x5f, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
	0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0c, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x53, 0x65,
	0x63, 0x72, 0x65, 0x74, 0x22, 0x46, 0x0a, 0x0a, 0x76, 0x65, 0x72, 0x69, 0x66, 0x79, 0x52, 0x65,
	0x73, 0x70, 0x12, 0x0e, 0x0a, 0x02, 0x69, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x02,
	0x69, 0x64, 0x12, 0x28, 0x0a, 0x04, 0x63, 0x6f, 0x64, 0x65, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0e,
	0x32, 0x14, 0x2e, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65,
	0x72, 0x2e, 0x43, 0x6f, 0x64, 0x65, 0x52, 0x04, 0x63, 0x6f, 0x64, 0x65, 0x2a, 0x52, 0x0a, 0x04,
	0x43, 0x6f, 0x64, 0x65, 0x12, 0x0f, 0x0a, 0x0b, 0x43, 0x4f, 0x44, 0x45, 0x5f, 0x55, 0x4e, 0x4b,
	0x4e, 0x4f, 0x57, 0x10, 0x00, 0x12, 0x0d, 0x0a, 0x09, 0x43, 0x4f, 0x44, 0x45, 0x5f, 0x50, 0x41,
	0x53, 0x53, 0x10, 0x01, 0x12, 0x0d, 0x0a, 0x09, 0x43, 0x4f, 0x44, 0x45, 0x5f, 0x46, 0x41, 0x49,
	0x4c, 0x10, 0x02, 0x12, 0x1b, 0x0a, 0x17, 0x43, 0x4f, 0x44, 0x45, 0x5f, 0x44, 0x45, 0x56, 0x49,
	0x43, 0x45, 0x49, 0x44, 0x5f, 0x4e, 0x4f, 0x54, 0x5f, 0x46, 0x4f, 0x55, 0x4e, 0x44, 0x10, 0x03,
	0x32, 0x54, 0x0a, 0x0f, 0x56, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72, 0x53, 0x65, 0x72, 0x76,
	0x69, 0x63, 0x65, 0x12, 0x41, 0x0a, 0x06, 0x76, 0x65, 0x72, 0x69, 0x66, 0x79, 0x12, 0x19, 0x2e,
	0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72, 0x2e, 0x76,
	0x65, 0x72, 0x69, 0x66, 0x79, 0x52, 0x65, 0x71, 0x1a, 0x1a, 0x2e, 0x64, 0x65, 0x76, 0x69, 0x63,
	0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72, 0x2e, 0x76, 0x65, 0x72, 0x69, 0x66, 0x79,
	0x52, 0x65, 0x73, 0x70, 0x22, 0x00, 0x42, 0x23, 0x5a, 0x21, 0x72, 0x74, 0x69, 0x6f, 0x32, 0x2f,
	0x70, 0x6b, 0x67, 0x2f, 0x72, 0x70, 0x63, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x2f, 0x64, 0x65, 0x76,
	0x69, 0x63, 0x65, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x65, 0x72, 0x62, 0x06, 0x70, 0x72, 0x6f,
	0x74, 0x6f, 0x33,
}

var (
	file_deviceverifier_deviceverifier_proto_rawDescOnce sync.Once
	file_deviceverifier_deviceverifier_proto_rawDescData = file_deviceverifier_deviceverifier_proto_rawDesc
)

func file_deviceverifier_deviceverifier_proto_rawDescGZIP() []byte {
	file_deviceverifier_deviceverifier_proto_rawDescOnce.Do(func() {
		file_deviceverifier_deviceverifier_proto_rawDescData = protoimpl.X.CompressGZIP(file_deviceverifier_deviceverifier_proto_rawDescData)
	})
	return file_deviceverifier_deviceverifier_proto_rawDescData
}

var file_deviceverifier_deviceverifier_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_deviceverifier_deviceverifier_proto_msgTypes = make([]protoimpl.MessageInfo, 2)
var file_deviceverifier_deviceverifier_proto_goTypes = []interface{}{
	(Code)(0),          // 0: deviceverifier.Code
	(*VerifyReq)(nil),  // 1: deviceverifier.verifyReq
	(*VerifyResp)(nil), // 2: deviceverifier.verifyResp
}
var file_deviceverifier_deviceverifier_proto_depIdxs = []int32{
	0, // 0: deviceverifier.verifyResp.code:type_name -> deviceverifier.Code
	1, // 1: deviceverifier.VerifierService.verify:input_type -> deviceverifier.verifyReq
	2, // 2: deviceverifier.VerifierService.verify:output_type -> deviceverifier.verifyResp
	2, // [2:3] is the sub-list for method output_type
	1, // [1:2] is the sub-list for method input_type
	1, // [1:1] is the sub-list for extension type_name
	1, // [1:1] is the sub-list for extension extendee
	0, // [0:1] is the sub-list for field type_name
}

func init() { file_deviceverifier_deviceverifier_proto_init() }
func file_deviceverifier_deviceverifier_proto_init() {
	if File_deviceverifier_deviceverifier_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_deviceverifier_deviceverifier_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*VerifyReq); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_deviceverifier_deviceverifier_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*VerifyResp); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_deviceverifier_deviceverifier_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   2,
			NumExtensions: 0,
			NumServices:   1,
		},
		GoTypes:           file_deviceverifier_deviceverifier_proto_goTypes,
		DependencyIndexes: file_deviceverifier_deviceverifier_proto_depIdxs,
		EnumInfos:         file_deviceverifier_deviceverifier_proto_enumTypes,
		MessageInfos:      file_deviceverifier_deviceverifier_proto_msgTypes,
	}.Build()
	File_deviceverifier_deviceverifier_proto = out.File
	file_deviceverifier_deviceverifier_proto_rawDesc = nil
	file_deviceverifier_deviceverifier_proto_goTypes = nil
	file_deviceverifier_deviceverifier_proto_depIdxs = nil
}
