#pragma once

#include "IPanel.h"

namespace vgui {
	enum VGUIPANEL {
		PANEL_ROOT ,
		PANEL_CLIENTDLL ,
		PANEL_GAMEUIDLL ,
	};

	class IEngineVGui : public IBaseInterface {
	public:
		virtual vgui::IPanel* GetPanel( VGUIPANEL type ) = 0;
	};
}

#define VENGINE_VGUI_VERSION x("VEngineVGui001")