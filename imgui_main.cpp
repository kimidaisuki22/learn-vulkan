#include "imgui_main.h"
#include "imgui.h"
#include <stdlib.h>

bool same_line_button(const char* text){
    ImGui::SameLine();
    return ImGui::Button(text);
}

void imgui_main(){
    if(ImGui::Button("open work dir")){
        system("start .");
    }
    if(same_line_button("open stack overflow")){
        system("start https://stackoverflow.com/");
    }
    if(same_line_button("power option")){
        system("start powercfg.cpl");
    }

}