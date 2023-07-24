'use strict';

//const vConsole = new VConsole();
//const remoteConsole = new RemoteConsole("http://[remote server]/logio-post");
//window.datgui = new dat.GUI();

var vue_options = {
    el: "#top",
    mixins: [mixins_bootstrap],
    data: {
        ipaddress: "",
        ir_list: [],
        params_ir_update: {},
        params_key_update: {},
        params_ir_register: {
            result: {}
        },
        keyid_list: keyid_list_jp,
    },
    computed: {
        host_url: function(){
            if( this.ipaddress )
                return "http://" + this.ipaddress;
            else
                return ""
        },
    },
    methods: {
        change_keylang: function(){
            if( this.params_key_update.key_lang == 1)
                this.keyid_list = keyid_list_jp;
            else
                this.keyid_list = keyid_list_us;
        },
        code2disp: function(code, key_lang){
            var item;
            if( key_lang == 1 )
                item = keyid_list_jp.find(item => item.keyid == code);
            else
                item = keyid_list_us.find(item => item.keyid == code);
            if( item )
                return item.disp;
            else
                return null;
        },
        mod2list: function(mod, mac){
            var list = [];
            var mod_list;
            if( mac )
                mod_list = ["Control", "Shift", "Option", "Command", "CapsLock", "Tab"];
            else
                mod_list = ["LEFT_CTRL", "LEFT_SHIFT", "LEFT_ALT", "LEFT_GUI", "RIGHT_CTRL", "RIGHT_SHIFT", "RIGHT_ALT", "RIGHT_GUI",];
            for( var i = 0 ; i < mod_list.length ; i++ )
                if( mod & (0x01 << i) )
                    list.push(mod_list[i]);
            return list;
        },

        ir_reboot: async function(){
            try{
                var params = {
                    endpoint: "/irhid-reboot",
                    params: {}
                };
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
        irlist_refresh: async function(){
            try{
                var params = {
                    endpoint: "/irhid-ir-list",
                    params: {}
                };
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";

                this.ir_list = result.result.ir_list;
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
        start_ir_update: async function(index){
            this.params_ir_update = {
                ir_value: this.ir_list[index].ir_value,
                ir_name: this.ir_list[index].ir_name
            };
            this.dialog_open("#ir_update_dialog");
        },
        call_ir_update: async function(){
            try{
                var params = {
                    endpoint: "/irhid-ir-update",
                    params: {
                        ir_value: this.params_ir_update.ir_value,
                        ir_name: this.params_ir_update.ir_name,
                    }
                }
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";

                this.dialog_close("#ir_update_dialog");
                this.irlist_refresh();
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
        start_ir_delete: async function(index){
            if( !confirm("本当に削除しますか？") )
                return;
            
            try{
                var params = {
                    endpoint: "/irhid-ir-delete",
                    params: {
                        ir_value: this.ir_list[index].ir_value
                    }
                }
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";

                this.irlist_refresh();
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
        start_key_update: async function(index){
            this.params_key_update = {
                ir_name: this.ir_list[index].ir_name,
                ir_value: this.ir_list[index].ir_value,
                key_name: this.ir_list[index].key_name,
                key_type: this.ir_list[index].key_type,
                key_lang: this.ir_list[index].key_lang,
                mods: [],
            };
            this.change_keylang();
            if( this.ir_list[index].key_type == 'text' ){
                this.params_key_update.text = this.ir_list[index].text;
            }else if( this.ir_list[index].key_type == 'key' ){
                var mod = this.ir_list[index].mod;
                if( mod & (0x01 << 0) ) this.params_key_update.mods.push("LEFT_CTRL");
                if( mod & (0x01 << 1) ) this.params_key_update.mods.push("LEFT_SHIFT");
                if( mod & (0x01 << 2) ) this.params_key_update.mods.push("LEFT_ALT");
                if( mod & (0x01 << 3) ) this.params_key_update.mods.push("LEFT_GUI");
                this.params_key_update.code = this.ir_list[index].code;
            }else if( this.ir_list[index].key_type == 'key_mac' ){
                var mod = this.ir_list[index].mod;
                if( mod & (0x01 << 0) ) this.params_key_update.mods.push("LEFT_CTRL");
                if( mod & (0x01 << 1) ) this.params_key_update.mods.push("LEFT_SHIFT");
                if( mod & (0x01 << 2) ) this.params_key_update.mods.push("LEFT_ALT");
                if( mod & (0x01 << 3) ) this.params_key_update.mods.push("LEFT_GUI");
                if( mod & (0x01 << 4) ) this.params_key_update.mods.push("LEFT_CAPS_LOCK");
                if( mod & (0x01 << 5) ) this.params_key_update.mods.push("LEFT_TAB");
                this.params_key_update.code = this.ir_list[index].code;
            }
            this.dialog_open("#key_update_dialog");
        },
        call_key_update: async function(){
            try{
                var params = {
                    endpoint: "/irhid-key-update",
                    params: {
                        ir_value: this.params_key_update.ir_value,
                        key_name: this.params_key_update.key_name,
                        key_type: this.params_key_update.key_type,
                        key_lang: this.params_key_update.key_lang,
                    }
                };
                if( this.params_key_update.key_type == 'text' ){
                    params.params.text = this.params_key_update.text;
                }else if( this.params_key_update.key_type == 'key' ){
                    params.params.mod = 0x00;
                    if( this.params_key_update.mods ){
                        if( this.params_key_update.mods.indexOf("LEFT_CTRL") >= 0 ) params.params.mod |= (0x01 << 0);
                        if( this.params_key_update.mods.indexOf("LEFT_SHIFT") >= 0 ) params.params.mod |= (0x01 << 1);
                        if( this.params_key_update.mods.indexOf("LEFT_ALT") >= 0 ) params.params.mod |= (0x01 << 2);
                        if( this.params_key_update.mods.indexOf("LEFT_GUI") >= 0 ) params.params.mod |= (0x01 << 3);
                    }
                    params.params.code = this.params_key_update.code;
                }else if( this.params_key_update.key_type == 'key_mac' ){
                    params.params.mod = 0x00;
                    if( this.params_key_update.mods ){
                        if( this.params_key_update.mods.indexOf("LEFT_CTRL") >= 0 ) params.params.mod |= (0x01 << 0);
                        if( this.params_key_update.mods.indexOf("LEFT_SHIFT") >= 0 ) params.params.mod |= (0x01 << 1);
                        if( this.params_key_update.mods.indexOf("LEFT_ALT") >= 0 ) params.params.mod |= (0x01 << 2);
                        if( this.params_key_update.mods.indexOf("LEFT_GUI") >= 0 ) params.params.mod |= (0x01 << 3);
                        if( this.params_key_update.mods.indexOf("LEFT_CAPS_LOCK") >= 0 ) params.params.mod |= (0x01 << 4);
                        if( this.params_key_update.mods.indexOf("LEFT_TAB") >= 0 ) params.params.mod |= (0x01 << 5);
                    }
                    params.params.code = this.params_key_update.code;
                }
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";

                this.dialog_close("#key_update_dialog");
                this.irlist_refresh();
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
        start_ir_register: async function(){
            try{
                this.progress_open("リモコンのボタンを押してください。");
                var params = {
                    endpoint: "/irhid-ir-check",
                    params: {}
                };
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);
                if( result.status != 'OK' )
                    throw "result.status not OK";

                var result;
                var start_tim = new Date().getTime();
                do{
                    var params = {
                        endpoint: "/irhid-ir-status",
                        params: {}
                    };
                    result = await do_post(this.host_url + "/endpoint", params);
                    console.log(result);
                    if( result.status != 'OK' )
                        throw "result.status not OK";                    
                    if( !result.result.irReceiving )
                        break;

                    var end_tim = new Date().getTime();
                    if( end_tim - start_tim >= 10000 )
                        break;
                    await wait_msec(1000);
                }while(true);
                if( !result.result.irReceived )
                    return;

                this.params_ir_register.name = "";
                this.params_ir_register.result = result.result;
                this.dialog_open('#ir_register_dialog');
            }catch(error){
                console.error(error);
                alert(error);
            }finally{
                this.progress_close();
            }
        },
        call_ir_register: async function(){
            try{
                var params = {
                    endpoint: "/irhid-ir-register",
                    params: {
                        ir_name: this.params_ir_register.name,
                        ir_value: this.params_ir_register.result.value
                    }
                };
                var result = await do_post(this.host_url + "/endpoint", params);
                console.log(result);

                this.dialog_close("#ir_register_dialog");
                this.irlist_refresh();
            }catch(error){
                console.error(error);
                alert(error);
            }
        },
    },
    created: function(){
    },
    mounted: function(){
        proc_load();
    }
};
vue_add_data(vue_options, { progress_title: '' }); // for progress-dialog
vue_add_global_components(components_bootstrap);
vue_add_global_components(components_utils);

/* add additional components */
  
window.vue = new Vue( vue_options );

async function wait_msec(msec){
    return new Promise(resolve => setTimeout(resolve, msec) );
}
