#include "amigaguide/document_editor.h"
#include "amigaguide/parser.h"
#include <cstdlib>
#include <iostream>
#include <string>
namespace { void check(bool c,const char* m){if(!c){std::cerr<<"FAIL: "<<m<<'\n';std::exit(1);}} amigaguide::Document parse(const std::string& s){amigaguide::Document d;amigaguide::ParseError e;check(amigaguide::Parser{}.parse(s,d,&e),"test source should parse");return d;} }
int main(){
 const std::string original="@database Test Guide\n@author Old Author\n@version 1.0\n@copyright Old Copyright\n@font Topaz 8\n@help Help\n@toc Main\n@index Index\n@worddelimiter \"-\"\n@width 640\n@height 480\n@tab 4\n@wordwrap\n\n@node Main \"Main Page\"\n@keywords old keywords\n@prev Previous\n@help \"Help Page\"\nPreserve this text.\n@{Keep LINK Keep}\n@endnode\n\n@node Keep \"Keep Page\"\nKeep this node.\n@endnode\n";
 auto document=parse(original);std::string error;amigaguide::DocumentEditor edit(document);
 check(edit.set_document_property(amigaguide::DocumentProperty::Name,"Changed Guide",&error),"name update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Author,"New Author",&error),"author update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Version,"2.0",&error),"version update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Copyright,"New Copyright",&error),"copyright update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Font,"Topaz 9",&error),"font update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::WordDelimiter,"|",&error),"word delimiter update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Width,"800",&error),"width update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Height,"600",&error),"height update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::TabWidth,"8",&error),"tab width update succeeds");
 check(edit.set_document_property(amigaguide::DocumentProperty::Index,"",&error),"index removal succeeds");
 check(edit.set_document_flag(amigaguide::DocumentFlag::SmartWrap,true,&error),"smartwrap insertion succeeds");
 auto reparsed=parse(document.source());
 check(reparsed.metadata().name=="Changed Guide","name parsed");check(reparsed.metadata().author=="New Author","author parsed");check(reparsed.metadata().version=="2.0","version parsed");check(reparsed.metadata().copyright=="New Copyright","copyright parsed");check(reparsed.metadata().font=="Topaz 9","font parsed");check(reparsed.metadata().word_delimiter=="|","word delimiter parsed");check(reparsed.metadata().width==800&&reparsed.metadata().height==600,"dimensions parsed");check(reparsed.metadata().tab_width==8,"tab width parsed");check(reparsed.metadata().smart_wrap,"smartwrap parsed");check(reparsed.source().find("@index") == std::string::npos,"index removed");check(reparsed.source().find("Preserve this text.")!=std::string::npos,"node content preserved");check(reparsed.source().find("@{Keep LINK Keep}")!=std::string::npos,"links preserved");
 amigaguide::DocumentEditor removeFlags(reparsed);check(removeFlags.set_document_flag(amigaguide::DocumentFlag::SmartWrap,false,&error),"smartwrap removal succeeds");auto noFlag=parse(reparsed.source());check(noFlag.source().find("@smartwrap")==std::string::npos,"smartwrap removed");
 amigaguide::DocumentEditor insert(parse("@node Main\n@endnode\n"));check(insert.set_document_property(amigaguide::DocumentProperty::Author,"Inserted",&error),"missing author insertion succeeds");check(insert.set_document_property(amigaguide::DocumentProperty::Width,"1024",&error),"missing width insertion succeeds");check(insert.set_document_flag(amigaguide::DocumentFlag::WordWrap,true,&error),"missing wordwrap insertion succeeds");auto inserted=parse(insert.source());check(inserted.metadata().author=="Inserted"&&inserted.metadata().width==1024,"inserted properties parse");
 std::cout<<"All document editor tests passed.\n";return 0;
}
