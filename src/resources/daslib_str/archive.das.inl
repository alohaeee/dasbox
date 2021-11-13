//
// AUTO-GENERATED FILE - DO NOT EDIT!!
//

"options indenting = 4\n"
"options no_unused_block_arguments = false\n"
"options no_unused_function_arguments = false\n"
"\n"
"module archive shared private\n"
"\n"
"require daslib/contracts\n"
"require daslib/apply\n"
"require daslib/templates\n"
"require math\n"
"require strings\n"
"require rtti\n"
"\n"
"class public Serializer\n"
"    def abstract write ( bytes:void? implicit; size:int ) : bool\n"
"    def abstract read ( bytes:void? implicit; size:int ) : bool\n"
"    def abstract error ( code:string ) : void\n"
"    def abstract OK : bool\n"
"\n"
"class public MemSerializer : Serializer\n"
"    def MemSerializer\n"
"        pass    // writing\n"
"    def MemSerializer ( from:array<uint8> )\n"
"        data := from\n"
"    def extractData : array<uint8>\n"
"        return <- data\n"
"    def getCopyOfData : array<uint8>\n"
"        var cdata := data\n"
"        return <- cdata\n"
"    def getLastError\n"
"        return lastError\n"
"    def override OK\n"
"        return lastError==\"\"\n"
"    def override write ( bytes:void? implicit; size:int ) : bool\n"
"        let pos = length(data)\n"
"        data |> resize(pos + size)\n"
"        unsafe\n"
"            memcpy ( addr(data[pos]), bytes, size)\n"
"        return true\n"
"    def override read ( bytes:void? implicit; size:int ) : bool\n"
"        let newOffset = readOffset + size\n"
"        let maxOffset = length(data)\n"
"        if newOffset > maxOffset\n"
"            error(\"reading past the end of stream\")\n"
"            readOffset = maxOffset\n"
"            return false\n"
"        unsafe\n"
"            memcpy ( bytes, addr(data[readOffset]), size)\n"
"        readOffset = newOffset\n"
"        return true\n"
"    def override error ( code:string ) : void\n"
"        lastError = code\n"
"    private data:array<uint8>\n"
"    private readOffset:int\n"
"    private lastError:string\n"
"\n"
"struct public Archive\n"
"    version : uint\n"
"    reading : bool\n"
"    stream  : Serializer?\n"
"\n"
"def public serialize_raw ( var arch:Archive; var value : auto(TT)& )\n"
"    if arch.reading\n"
"        arch.stream->read(unsafe(addr(value)), typeinfo(sizeof type<TT-&>))\n"
"    else\n"
"        arch.stream->write(unsafe(addr(value)), typeinfo(sizeof type<TT-&>))\n"
"\n"
"def public read_raw ( var arch:Archive; var value : auto(TT)& )\n"
"    assert(arch.reading)\n"
"    arch.stream->read(unsafe(addr(value)), typeinfo(sizeof type<TT-&>))\n"
"\n"
"def public write_raw ( var arch:Archive; var value : auto(TT)& )\n"
"    assert(!arch.reading)\n"
"    arch.stream->write(unsafe(addr(value)), typeinfo(sizeof type<TT-&>))\n"
"\n"
"[expect_any_enum(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value : auto(TT)&; dummy:TT-&??????"
" = null )\n"
"    arch |> serialize_raw(value)\n"
"\n"
"[expect_any_function(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value:auto(TT)&; dummy:TT-&????? = "
"null )\n"
"    if arch.reading\n"
"        var mnh : uint\n"
"        arch |> read_raw(mnh)\n"
"        if mnh != 0u\n"
"            unsafe\n"
"                value = reinterpret<TT-#>(get_function_by_mangled_name_hash(mnh)"
")\n"
"    else\n"
"        var mnh = get_function_mangled_name_hash(value)\n"
"        arch |> write_raw(mnh)\n"
"\n"
"[expect_any_workhorse_raw(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value : auto(TT)&; dummy:TT-&???? ="
" null )\n"
"    arch |> serialize_raw(value)\n"
"\n"
"def public serialize ( var arch:Archive; var value : float3x3 )\n"
"    arch |> serialize_raw(value)\n"
"\n"
"def public serialize ( var arch:Archive; var value : float3x4 )\n"
"    arch |> serialize_raw(value)\n"
"\n"
"def public serialize ( var arch:Archive; var value : float4x4 )\n"
"    arch |> serialize_raw(value)\n"
"\n"
"[expect_any_struct(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value : auto(TT)&; dummy:TT-&? = nu"
"ll )\n"
"    if arch.reading\n"
"        delete value\n"
"    apply(value) <| $ ( name:string; field )\n"
"        arch |> _::serialize(field)\n"
"\n"
"[expect_any_tuple(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value : auto(TT)&; dummy:TT-&?? = n"
"ull )\n"
"    if arch.reading\n"
"        delete value\n"
"    apply(value) <| $ ( name:string; field )\n"
"        arch |> _::serialize(field)\n"
"\n"
"[expect_any_variant(value), template(dummy=false)]\n"
"def public serialize ( var arch:Archive; var value : auto(TT)&; dummy:TT-&??? = "
"null )\n"
"    if arch.reading\n"
"        delete value\n"
"        var index : int\n"
"        arch |> read_raw(index)\n"
"        unsafe\n"
"            value |> set_variant_index(index)\n"
"    else\n"
"        var index = variant_index(value)\n"
"        arch |> write_raw(index)\n"
"    apply(value) <| $ ( name:string; field )\n"
"        arch |> _::serialize(field)\n"
"\n"
"def public serialize ( var arch:Archive; var value : auto(TT)[] )\n"
"    for element in value\n"
"        arch |> _::serialize(element)\n"
"\n"
"def public serialize ( var arch:Archive; var value : array<auto(TT)> )\n"
"    if arch.reading\n"
"        var len : int\n"
"        delete value\n"
"        arch |> read_raw(len)\n"
"        value |> resize(len)\n"
"        for element in value\n"
"            arch |> _::serialize(element)\n"
"    else\n"
"        var len = length(value)\n"
"        arch |> write_raw(len)\n"
"        for element in value\n"
"            arch |> _::serialize(element)\n"
"\n"
"def public serialize ( var arch:Archive; var value : table<auto(KT);auto(VT)> )\n"
"    if arch.reading\n"
"        var len : int\n"
"        arch |> read_raw(len)\n"
"        delete value\n"
"        for i in range(len)\n"
"            var k : KT-const-&-#\n"
"            arch |> _::serialize(k)\n"
"            arch |> _::serialize(value[k])\n"
"    else\n"
"        var len = length(value)\n"
"        arch |> write_raw(len)\n"
"        for k,v in keys(value),values(value)\n"
"            var K = k\n"
"            arch |> _::serialize(K)\n"
"            arch |> _::serialize(v)\n"
"\n"
"def public serialize ( var arch:Archive; var value : string& )\n"
"    unsafe\n"
"        if arch.reading\n"
"            var len: int\n"
"            var tbuf: array<uint8>\n"
"            arch |> read_raw(len)\n"
"            tbuf |> resize(len+1)                           // todo: implement p"
"reallocat string with raii\n"
"            arch.stream->read(addr(tbuf[0]), len+1)\n"
"            let tstr = reinterpret<string#> addr(tbuf[0])\n"
"            value := tstr\n"
"            delete tbuf\n"
"        else\n"
"            var len = length(value)\n"
"            arch |> write_raw(len)\n"
"            arch.stream->write(reinterpret<void?> value, len+1)\n"
"\n"
"def public serialize ( var arch:Archive; var value:auto(TT)? )\n"
"    if arch.reading\n"
"        value = null\n"
"\n"
"def public mem_archive_save ( var t:auto& )\n"
"    var mems = new MemSerializer()\n"
"    var sarch = [[Archive reading=false, stream=mems]]\n"
"    sarch |> serialize(t)\n"
"    return <- mems->extractData()\n"
"\n"
"def public mem_archive_load ( var data:array<uint8>; var t:auto&; canfail:bool=f"
"alse ) : bool\n"
"    var memr = new MemSerializer(data)\n"
"    var rarch = [[Archive reading=true, stream=memr]]\n"
"    rarch |> serialize(t)\n"
"    if !memr->OK()\n"
"        if !canfail\n"
"            panic(memr->getLastError())\n"
"        return false\n"
"    return true\n"
