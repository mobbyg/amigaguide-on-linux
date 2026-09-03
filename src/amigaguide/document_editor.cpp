#include "amigaguide/document_editor.h"

#include <algorithm>
#include <cctype>

namespace amigaguide {
namespace {
struct Token { std::size_t begin = 0; std::size_t end = 0; bool quoted = false; };
bool next_token(std::string_view value, std::size_t& cursor, Token& token){while(cursor<value.size()&&std::isspace(static_cast<unsigned char>(value[cursor])))++cursor;if(cursor>=value.size())return false;token.begin=cursor;token.quoted=value[cursor]=='"';if(token.quoted){++cursor;while(cursor<value.size()&&value[cursor]!='"')++cursor;if(cursor<value.size())++cursor;token.end=cursor;return true;}while(cursor<value.size()&&!std::isspace(static_cast<unsigned char>(value[cursor])))++cursor;token.end=cursor;return true;}
std::string quote(std::string_view value){std::string result="\"";for(const char c:value)result+=c=='"'?"\\\"":std::string(1,c);result+='"';return result;}
std::string lower(std::string_view value){std::string result(value);for(char& c:result)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return result;}
bool is_title_command(std::string_view line,Token& argument){std::size_t cursor=0;Token command;if(!next_token(line,cursor,command))return false;if(lower(line.substr(command.begin,command.end-command.begin))!="@title")return false;return next_token(line,cursor,argument);}
const char* property_command(NodeProperty property){switch(property){case NodeProperty::Keywords:return "@keywords";case NodeProperty::Prev:return "@prev";case NodeProperty::Next:return "@next";case NodeProperty::Help:return "@help";case NodeProperty::Toc:return "@toc";case NodeProperty::Index:return "@index";case NodeProperty::Font:return "@font";case NodeProperty::TabWidth:return "@tab";}return "";}
const char* flag_command(NodeFlag flag){switch(flag){case NodeFlag::WordWrap:return "@wordwrap";case NodeFlag::SmartWrap:return "@smartwrap";case NodeFlag::Proportional:return "@proportional";}return "";}
const char* document_property_command(DocumentProperty property){switch(property){case DocumentProperty::Name:return "@database";case DocumentProperty::Author:return "@author";case DocumentProperty::Version:return "@version";case DocumentProperty::Copyright:return "@copyright";case DocumentProperty::Font:return "@font";case DocumentProperty::Help:return "@help";case DocumentProperty::Toc:return "@toc";case DocumentProperty::Index:return "@index";case DocumentProperty::WordDelimiter:return "@worddelimiter";case DocumentProperty::Width:return "@width";case DocumentProperty::Height:return "@height";case DocumentProperty::TabWidth:return "@tab";}return "";}
bool command_matches(std::string_view line,std::string_view wanted,Token* argument=nullptr){std::size_t cursor=0;Token command;if(!next_token(line,cursor,command))return false;if(lower(line.substr(command.begin,command.end-command.begin))!=lower(wanted))return false;if(argument)return next_token(line,cursor,*argument);return true;}
}
void DocumentEditor::set_error(std::string* error,std::string message){if(error)*error=std::move(message);}
bool DocumentEditor::replace_source(std::size_t begin,std::size_t end,std::string_view replacement,std::string* error){if(begin>end||end>document_.source().size()){set_error(error,"invalid source range");return false;}document_.source().replace(begin,end-begin,replacement.data(),replacement.size());return true;}

bool DocumentEditor::set_document_property(DocumentProperty property,std::string value,std::string* error){
    const std::string command=document_property_command(property);
    const auto first_node=document_.nodes().empty()?document_.source().size():document_.nodes().front().source_begin;
    std::size_t line_begin=0;
    while(line_begin<first_node){
        const auto line_end=document_.source().find('\n',line_begin);
        const auto end=line_end==std::string::npos?first_node:std::min(line_end,first_node);
        const std::string_view line(document_.source().data()+line_begin,end-line_begin);
        Token argument;
        if(command_matches(line,command,&argument)){
            if(value.empty()){const auto remove_end=line_end==std::string::npos?end:line_end+1;return replace_source(line_begin,remove_end,{},error);}
            std::string replacement=(property==DocumentProperty::WordDelimiter||property==DocumentProperty::Width||property==DocumentProperty::Height||property==DocumentProperty::TabWidth)?value:quote(value);
            return replace_source(line_begin+argument.begin,line_begin+argument.end,replacement,error);
        }
        if(line_end==std::string::npos||line_end>=first_node)break;line_begin=line_end+1;
    }
    if(value.empty())return true;
    const std::string replacement=std::string(command)+" "+((property==DocumentProperty::WordDelimiter||property==DocumentProperty::Width||property==DocumentProperty::Height||property==DocumentProperty::TabWidth)?value:quote(value))+"\n";
    return replace_source(first_node,first_node,replacement,error);
}

bool DocumentEditor::set_document_flag(DocumentFlag flag,bool enabled,std::string* error){
    const char* command=flag==DocumentFlag::WordWrap?"@wordwrap":"@smartwrap";
    const auto first_node=document_.nodes().empty()?document_.source().size():document_.nodes().front().source_begin;
    std::size_t line_begin=0;
    while(line_begin<first_node){
        const auto line_end=document_.source().find('\n',line_begin);
        const auto end=line_end==std::string::npos?first_node:std::min(line_end,first_node);
        const std::string_view line(document_.source().data()+line_begin,end-line_begin);
        if(command_matches(line,command)){
            if(!enabled){const auto remove_end=line_end==std::string::npos?end:line_end+1;return replace_source(line_begin,remove_end,{},error);}return true;
        }
        if(line_end==std::string::npos||line_end>=first_node)break;line_begin=line_end+1;
    }
    if(!enabled)return true;return replace_source(first_node,first_node,std::string(command)+"\n",error);
}

bool DocumentEditor::add_node(std::string name,std::string title,std::string* error){if(name.empty()){set_error(error,"node name cannot be empty");return false;}if(document_.find_node(name)){set_error(error,"a node with that name already exists");return false;}std::string block;if(!document_.source().empty()&&document_.source().back()!='\n')block+='\n';block+="@node "+name+" "+quote(title)+"\n\n@endnode\n";document_.source().append(block);return true;}
bool DocumentEditor::rename_node(std::size_t index,std::string name,std::string* error){if(index>=document_.nodes().size()){set_error(error,"node index is out of range");return false;}if(name.empty()){set_error(error,"node name cannot be empty");return false;}const auto& node=document_.nodes()[index];const Node* existing=document_.find_node(name);if(existing&&existing!=&node){set_error(error,"a node with that name already exists");return false;}const auto line_end=document_.source().find('\n',node.source_begin);const auto header_end=line_end==std::string::npos?document_.source().size():line_end;const std::string_view line(document_.source().data()+node.source_begin,header_end-node.source_begin);std::size_t cursor=0;Token command,name_token;if(!next_token(line,cursor,command)||!next_token(line,cursor,name_token)){set_error(error,"could not locate node declaration");return false;}const std::string replacement=name_token.quoted?quote(name):name;return replace_source(node.source_begin+name_token.begin,node.source_begin+name_token.end,replacement,error);}
bool DocumentEditor::set_node_title(std::size_t index,std::string title,std::string* error){if(index>=document_.nodes().size()){set_error(error,"node index is out of range");return false;}const auto& node=document_.nodes()[index];const std::size_t block_end=node.source_end;std::size_t line_begin=node.source_begin;while(line_begin<block_end){const auto line_end=document_.source().find('\n',line_begin);const auto end=line_end==std::string::npos?block_end:std::min(line_end,block_end);const std::string_view line(document_.source().data()+line_begin,end-line_begin);Token argument;if(is_title_command(line,argument))return replace_source(line_begin+argument.begin,line_begin+argument.end,quote(title),error);if(line_end==std::string::npos||line_end>=block_end)break;line_begin=line_end+1;}const auto header_end=document_.source().find('\n',node.source_begin);const auto end=header_end==std::string::npos?document_.source().size():header_end;const std::string_view line(document_.source().data()+node.source_begin,end-node.source_begin);std::size_t cursor=0;Token command,name_token;if(!next_token(line,cursor,command)||!next_token(line,cursor,name_token)){set_error(error,"could not locate node declaration");return false;}Token title_token;if(next_token(line,cursor,title_token))return replace_source(node.source_begin+title_token.begin,node.source_begin+title_token.end,quote(title),error);return replace_source(end,end,std::string(" ")+quote(title),error);}
bool DocumentEditor::set_node_property(std::size_t index,NodeProperty property,std::string value,std::string* error){if(index>=document_.nodes().size()){set_error(error,"node index is out of range");return false;}const auto& node=document_.nodes()[index];const std::string command=property_command(property);std::size_t line_begin=node.source_begin;while(line_begin<node.source_end){const auto line_end=document_.source().find('\n',line_begin);const auto end=line_end==std::string::npos?node.source_end:std::min(line_end,node.source_end);const std::string_view line(document_.source().data()+line_begin,end-line_begin);Token argument;if(command_matches(line,command,&argument)){if(value.empty()){const auto remove_end=line_end==std::string::npos?end:line_end+1;return replace_source(line_begin,remove_end,{},error);}std::string replacement=(property==NodeProperty::Keywords||property==NodeProperty::TabWidth)?value:quote(value);return replace_source(line_begin+argument.begin,line_begin+argument.end,replacement,error);}if(line_end==std::string::npos||line_end>=node.source_end)break;line_begin=line_end+1;}if(value.empty())return true;const std::string formatted=command+" "+((property==NodeProperty::Keywords||property==NodeProperty::TabWidth)?value:quote(value))+"\n";const auto endnode=document_.source().rfind("@endnode",node.source_end);if(endnode==std::string::npos||endnode<node.source_begin){set_error(error,"could not locate @ENDNODE");return false;}return replace_source(endnode,endnode,formatted,error);}
bool DocumentEditor::set_node_flag(std::size_t index,NodeFlag flag,bool enabled,std::string* error){if(index>=document_.nodes().size()){set_error(error,"node index is out of range");return false;}const auto& node=document_.nodes()[index];const std::string command=flag_command(flag);std::size_t line_begin=node.source_begin;while(line_begin<node.source_end){const auto line_end=document_.source().find('\n',line_begin);const auto end=line_end==std::string::npos?node.source_end:std::min(line_end,node.source_end);const std::string_view line(document_.source().data()+line_begin,end-line_begin);if(command_matches(line,command)){if(!enabled){const auto remove_end=line_end==std::string::npos?end:line_end+1;return replace_source(line_begin,remove_end,{},error);}return true;}if(line_end==std::string::npos||line_end>=node.source_end)break;line_begin=line_end+1;}if(!enabled)return true;const auto endnode=document_.source().rfind("@endnode",node.source_end);if(endnode==std::string::npos||endnode<node.source_begin){set_error(error,"could not locate @ENDNODE");return false;}return replace_source(endnode,endnode,std::string(command)+"\n",error);}
bool DocumentEditor::remove_node(std::size_t index,std::string* error){if(index>=document_.nodes().size()){set_error(error,"node index is out of range");return false;}const auto& node=document_.nodes()[index];if(node.source_end<=node.source_begin||node.source_end>document_.source().size()){set_error(error,"could not locate complete node source");return false;}return replace_source(node.source_begin,node.source_end,{},error);}
} // namespace amigaguide
