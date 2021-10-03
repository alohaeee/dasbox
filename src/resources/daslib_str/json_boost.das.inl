//
// AUTO-GENERATED FILE - DO NOT EDIT!!
//

"options no_unused_block_arguments = false\n"
"options no_unused_function_arguments = false\n"
"options indenting = 4\n"
"\n"
"module json_boost shared public\n"
"\n"
"require rtti\n"
"require ast\n"
"require strings\n"
"require daslib/json public\n"
"require daslib/ast_boost\n"
"require daslib/contracts\n"
"require daslib/templates\n"
"require daslib/strings_boost\n"
"require daslib/apply\n"
"\n"
"def private is_json_ptr_value ( td : TypeDeclPtr )\n"
"    if td.baseType != Type tPointer\n"
"        return false\n"
"    if td.firstType == null\n"
"        return false\n"
"    if td.firstType.baseType != Type tStructure\n"
"        return false\n"
"    let st = td.firstType.structType\n"
"    if !eq(st.name,\"JsonValue\") && !eq(st._module.name,\"json\")\n"
"        return false\n"
"    return true\n"
"\n"
"\n"
"// replacing ExprIsVariant(value,name) => ExprIsVariant(value.value,name)\n"
"// replacing ExprAsVariant(value,name) => ExprAsVariant(value.value,name)\n"
"// if value is json::JsonValue?\n"
"[variant_macro(name=\"better_json\")]\n"
"class private BetterJsonMacro : AstVariantMacro\n"
"    def override visitExprIsVariant(prog:ProgramPtr; mod:Module?; expr:smart_ptr"
"<ExprIsVariant>) : ExpressionPtr\n"
"        if is_json_ptr_value(expr.value._type)\n"
"            var vdr <- new [[ExprField() at=expr.at, name:=\"value\", value <- clo"
"ne_expression(expr.value)]]\n"
"            var isv <- new [[ExprIsVariant() at=expr.at, name:=expr.name, value "
"<- vdr ]]\n"
"            return isv\n"
"        return [[ExpressionPtr]]\n"
"    def override visitExprAsVariant(prog:ProgramPtr; mod:Module?; expr:smart_ptr"
"<ExprAsVariant>) : ExpressionPtr\n"
"        if is_json_ptr_value(expr.value._type)\n"
"            var vdr <- new [[ExprField() at=expr.at, name:=\"value\", value <- clo"
"ne_expression(expr.value)]]\n"
"            var isv <- new [[ExprAsVariant() at=expr.at, name:=expr.name, value "
"<- vdr ]]\n"
"            return isv\n"
"        return [[ExpressionPtr]]\n"
"\n"
"[reader_macro(name=\"json\")]\n"
"class private JsonReader : AstReaderMacro\n"
"    def override accept ( prog:ProgramPtr; mod:Module?; var expr:ExprReader?; ch"
":int; info:LineInfo ) : bool\n"
"        append(expr.sequence,ch)\n"
"        if ends_with(expr.sequence,\"%%\")\n"
"            let len = length(expr.sequence)\n"
"            resize(expr.sequence,len-2)\n"
"            return false\n"
"        else\n"
"            return true\n"
"    def override visit ( prog:ProgramPtr; mod:Module?; expr:smart_ptr<ExprReader"
"> ) : ExpressionPtr\n"
"        let seqStr = string(expr.sequence)\n"
"        var error = \"\"\n"
"        var json = read_json(seqStr, error)\n"
"        if json == null\n"
"            macro_error(prog,expr.at,\"json did not parse ({error})\")\n"
"            return [[ExpressionPtr]]\n"
"        else\n"
"            var exx <- convert_to_expression(json,expr.at)\n"
"            return exx\n"
"\n"
"[template(ent)]\n"
"def from_JV ( v:JsonValue?; ent : auto(EnumT); defV : EnumT = [[EnumT]] ) : Enum"
"T\n"
"    if v==null || !(v.value is _string)\n"
"        return defV\n"
"    let name = v.value as _string\n"
"    var res : auto(EnumTT) = [[EnumT]]\n"
"    var ti = typeinfo(rtti_typeinfo type<EnumT>)\n"
"    for ef in *ti.enumType\n"
"        if name==ef.name\n"
"            unsafe\n"
"                res = reinterpret<EnumTT> (ef.value)\n"
"            return res\n"
"    panic(\"not a valid enumeration {name} in {typeinfo(typename type<EnumT>)}\")\n"
"    return res\n"
"\n"
"[generic,template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent:string; defV:string=\"\" )\n"
"    if v==null || !(v.value is _string)\n"
"        return defV\n"
"    else\n"
"        return v.value as _string\n"
"\n"
"[generic,template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent:bool; defV:bool=false )\n"
"    if v==null || !(v.value is _bool)\n"
"        return defV\n"
"    else\n"
"        return v.value as _bool\n"
"\n"
"[generic,template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent:float; defV:float=0. )\n"
"    if v==null || !(v.value is _number)\n"
"        return defV\n"
"    else\n"
"        return float(v.value as _number)\n"
"\n"
"[generic,template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent:int; defV:int=0 )\n"
"    if v==null || !(v.value is _number)\n"
"        return defV\n"
"    else\n"
"        return int(v.value as _number)\n"
"\n"
"[generic,template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent:bitfield; defV:bitfield=[[bitfield]] )\n"
"    if v==null || !(v.value is _number)\n"
"        return defV\n"
"    else\n"
"        return bitfield(v.value as _number)\n"
"\n"
"[expect_any_vector_type(v)]\n"
"def JV ( v : auto(VecT) )\n"
"    static_if typeinfo(vector_dim type<VecT>)==2\n"
"        return JV({{\"x\" => JV(v.x); \"y\" => JV(v.y)}})\n"
"    static_elif typeinfo(vector_dim type<VecT>)==3\n"
"        return JV({{\"x\" => JV(v.x); \"y\" => JV(v.y); \"z\" => JV(v.z)}})\n"
"    static_elif typeinfo(vector_dim type<VecT>)==4\n"
"        return JV({{\"x\" => JV(v.x); \"y\" => JV(v.y); \"z\" => JV(v.z); \"w\" => JV(v."
"w)}})\n"
"\n"
"[expect_any_vector_type(ent), template(ent)]\n"
"def from_JV ( var v : JsonValue?; ent: auto(VecT); defV:VecT=[[VecT]]  )\n"
"    if v==null\n"
"        return defV\n"
"    let VecV : auto(VecTT) = VecT()\n"
"    var vo & = v.value as _object\n"
"    static_if typeinfo(vector_dim VecV)==2\n"
"        return VecTT(\n"
"            vo[\"x\"].value as _number,\n"
"            vo[\"y\"].value as _number)\n"
"    static_elif typeinfo(vector_dim VecV)==3\n"
"        return VecTT(\n"
"            vo[\"x\"].value as _number,\n"
"            vo[\"y\"].value as _number,\n"
"            vo[\"z\"].value as _number)\n"
"    static_elif typeinfo(vector_dim VecV)==4\n"
"        return VecTT(\n"
"            vo[\"x\"].value as _number,\n"
"            vo[\"y\"].value as _number,\n"
"            vo[\"z\"].value as _number,\n"
"            vo[\"w\"].value as _number)\n"
"\n"
"\n"
"def JV ( v : float )\n"
"    return new [[JsonValue value <- [[JsValue _number = double(v)]]]]\n"
"\n"
"def JV ( v : int )\n"
"    return new [[JsonValue value <- [[JsValue _number = double(v)]]]]\n"
"\n"
"def JV ( v : bitfield )\n"
"    return new [[JsonValue value <- [[JsValue _number = double(v)]]]]\n"
"\n"
"def JV(val: int8): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: uint8): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: int16): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: uint16): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: uint): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: int64): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"def JV(val: uint64): JsonValue?\n"
"    return new [[JsonValue value <- [[JsValue _number = double(val)]]]]\n"
"\n"
"\n"
"def JV(value): JsonValue?\n"
"    static_if typeinfo(typename value) == \"JVon::JsonValue?\" || typeinfo(typenam"
"e value) == \"JVon::JsonValue? const\"\n"
"        return value\n"
"    static_elif typeinfo(is_pointer value)\n"
"        return value == null ? null : _::JV(*value)\n"
"    static_elif typeinfo(is_enum value)\n"
"        return JV(\"{value}\")\n"
"    static_elif typeinfo(is_string value)\n"
"        return _::JV(value)\n"
"    static_elif typeinfo(is_struct value)\n"
"        var map: table<string; JsonValue?>\n"
"        apply(value) <| $(name: string; field)\n"
"            static_if typeinfo(is_function field)\n"
"                pass // skip functions\n"
"            static_elif typeinfo(is_void_pointer field)\n"
"                pass // skip void?\n"
"            static_elif typeinfo(is_pointer field)\n"
"                if field != null\n"
"                    map[name] = _::JV(field)\n"
"            else\n"
"                map[name] = _::JV(field)\n"
"        return _::JV(map)\n"
"    static_elif typeinfo(is_table value)\n"
"        var map: table<string; JsonValue?>\n"
"        for k, v in keys(value), values(value)\n"
"            map[k] = _::JV(v)\n"
"        return _::JV(map)\n"
"    static_elif typeinfo(is_iterable value)\n"
"        var arr: array<JsonValue?>\n"
"        for x in value\n"
"            arr |> push <| _::JV(x)\n"
"        return _::JV(arr)\n"
"    else\n"
"        print(\"unsupported type {typeinfo(typename value)}\\n\")\n"
"        concept_assert(false, \"unknown JV()\")\n"
"        return null\n"
"\n"
"\n"
"def JV(val1; val2): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2)}])\n"
"\n"
"def JV(val1; val2; val3): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3)}])\n"
"\n"
"def JV(val1; val2; val3; val4): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5; val6): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5); _::JV(val6)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5; val6; val7): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5); _::JV(val6); _::JV(val7)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5; val6; val7; val8): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5); _::JV(val6); _::JV(val7); _::JV(val8)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5; val6; val7; val8; val9): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5); _::JV(val6); _::JV(val7); _::JV(val8); _::JV(val9)}])\n"
"\n"
"def JV(val1; val2; val3; val4; val5; val6; val7; val8; val9; val10): JsonValue?\n"
"    return _::JV([{auto[] _::JV(val1); _::JV(val2); _::JV(val3); _::JV(val4); _:"
":JV(val5); _::JV(val6); _::JV(val7); _::JV(val8); _::JV(val9); _::JV(val10)}])\n"