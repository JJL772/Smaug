#include "uiview.h"
#include "objexporter.h"
#include "vmfexporter.h"
#include "filesystem.h"
#include "basetool.h"
#include "cursor.h"
#include "nodetools.h"
#include "texturebrowser.h"
#include "grid.h"
#include "utils.h"
#include "worldsave.h"

#include <imgui_internal.h>

void CUIView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	//ImGui::CreateNewWindow

	// Move this?
	GetSettingsRegister().LoadSettings();

	m_editViews[0].m_editPlaneAngle = { 0, 0, 0 };
	m_editViews[1].m_editPlaneAngle = { -PI / 2, 0, 0 };
	m_editViews[2].m_editPlaneAngle = { -PI / 2, PI / 2, 0 };

	for(int i = 0; i < 3; i++)
		m_editViews[i].Init(ViewID::EDIT_VIEW + i, 1024, 1024, 0x121212FF);
	m_previewView.Init(ViewID::PREVIEW_VIEW, 1024, 1024, 0x383838FF);
	SelectedView().Init(ViewID::SELECTED_VIEW, 1024, 1024, 0x383838FF);

	m_drawPreviewView = true;
	m_drawEditView = true;
	m_drawSelectedView = true;

	m_toolBox.RegisterTool(new CDragTool());
	m_toolBox.RegisterTool(new CExtrudeTool());

}

void CUIView::Draw(float dt)
{
	CBaseView::Draw(dt);



	for (int i = 0; i < 3; i++)
		m_editViews[i].Draw(dt);
	if (m_drawPreviewView)
		m_previewView.Draw(dt);
	if (m_drawSelectedView)
		SelectedView().Draw(dt);


}

void CUIView::Update(float dt, float mx, float my)
{
	GetCursor().Update(dt);
	ImVec2 mv = ImGui::GetMousePos();

	// In Smaug, Y+ is up Z+ is forward and X+ is right
	ImVec2 uv0, uv1;
	uv0.x = 1;
	uv1.x = 0;
	if (RendererProperties().coordSystem == ECoordSystem::RIGHT_HANDED)
	{
		uv0.y = 1;
		uv1.y = 0;
	}
	else
	{
		uv0.y = 0;
		uv1.y = 1;
	}


	// UI
	ImGui::ShowDemoWindow();

	bool openNewFilePrompt = false;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			openNewFilePrompt = ImGui::MenuItem("New");

			if (ImGui::MenuItem("Save"))
			{
				char* str = saveWorld();
				filesystem::SaveFileWithDialog(str, "*.smf");
				delete[] str;
			}

			if (ImGui::MenuItem("Load"))
			{
				size_t len;
				char* str = filesystem::LoadFileWithDialog(len, "*.smf");
				if (str)
				{
					if(*str)
						loadWorld(str);
					delete[] str;
				}
			}

			if (ImGui::BeginMenu("Export"))
			{
				if (ImGui::MenuItem("Export to VMF"))
				{
					CVMFExporter o;
					char* str = o.Export(&GetWorldEditor());
					filesystem::SaveFileWithDialog(str, "*.vmf");
					delete[] str;
				}

				if (ImGui::MenuItem("Export to OBJ"))
				{
					COBJExporter o;
					char* str = o.Export(&GetWorldEditor());
					filesystem::SaveFileWithDialog(str, "*.obj");
					delete[] str;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Settings"))
			{
				m_settingsMenu.Enable();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}


	if(openNewFilePrompt)
		ImGui::OpenPopup("NewFilePrompt");

	if (ImGui::BeginPopupModal("NewFilePrompt", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Clear the world and create a new file?\n");

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			defaultWorld();
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	for (int i = 0; i < 3; i++)
	{
		if(ImGui::Begin(i == 0 ? "2D Editor" : i == 1 ? "2D Editor 2" : "2D Editor 3"))
		{
			m_drawEditView = !ImGui::IsWindowCollapsed();
	
			ImVec2 imageSize = ImGui::GetContentRegionAvail();
			m_editViews[i].m_aspectRatio = imageSize.x / imageSize.y;

			ImGui::Image(m_editViews[i].GetImTextureID(), imageSize, uv0, uv1);

			bool hoveredOn2DEditor = ImGui::IsItemHovered();
			ImVec2 inv = ImGui::GetItemRectMin();
			ImVec2 ixv = ImGui::GetItemRectMax();
		

			if (m_drawEditView && hoveredOn2DEditor && !m_previewView.m_controllingCamera)
			{
				float x = (mv.x - inv.x) / (ixv.x - inv.x);
				float y = (mv.y - inv.y) / (ixv.y - inv.y);

				m_editViews[i].m_focused = true;
				m_editViews[i].Update(dt, x, y);
				m_toolBox.Update(dt, m_editViews[i].TransformMousePos(x, y));

			}
			else
				m_editViews[i].m_focused = false;
		}
		ImGui::End();
	}

	if(ImGui::Begin("3D Preview"))
	{
		m_drawPreviewView = !ImGui::IsWindowCollapsed();
		
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_previewView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_previewView.GetImTextureID(), imageSize, uv0, uv1);
		
		bool hoveredOn3DPreview = ImGui::IsItemHovered();
		ImVec2 inv = ImGui::GetItemRectMin();
		ImVec2 ixv = ImGui::GetItemRectMax();
		
		if (m_drawPreviewView && (hoveredOn3DPreview || m_previewView.m_controllingCamera))
		{
			float x = (mv.x - inv.x) / (ixv.x - inv.x);
			float y = (mv.y - inv.y) / (ixv.y - inv.y);

			m_previewView.m_focused = true;
			m_previewView.Update(dt, x, y);
		}
		else
			m_previewView.m_focused = false;
	}
	ImGui::End();

	m_drawSelectedView = SelectedView().Show();

	GetActionManager().Update();

	 
	m_toolBox.ShowToolBox();
	m_settingsMenu.DrawMenu();
	TextureBrowser().Show();
	
	Grid().Update();

}
