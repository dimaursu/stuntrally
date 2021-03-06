#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/GuiCom.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/MultiList2.h"
#include "common/Slider.h"
#include "common/Gui_Popup.h"
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//---------------------------------------------------------------------------------------------------------------------

void CGui::InitGui()
{
	mGui = app->mGui;
	gcom->mGui = mGui;
	Check::pGUI = mGui;  SliderValue::pGUI = mGui;
	Check::bGI = &bGI;   SliderValue::bGI = &bGI;

	popup->mGui = mGui;
	popup->mPlatform = app->mPlatform;

	if (!mGui)  return;
	QTimer ti;  ti.update();  /// time


	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");

	//  load
	app->vwGui = LayoutManager::getInstance().loadLayout("Game.layout");


	//  wnds
	app->mWndMain = fWnd("MainMenuWnd");
	app->mWndGame = fWnd("GameWnd");
	app->mWndReplays = fWnd("ReplaysWnd");
	app->mWndHelp = fWnd("HelpWnd");
	app->mWndOpts = fWnd("OptionsWnd");

	app->mWndChampStage = fWnd("WndChampStage");  app->mWndChampStage->setVisible(false);
	app->mWndChampEnd   = fWnd("WndChampEnd");    app->mWndChampEnd->setVisible(false);
	app->mWndChallStage = fWnd("WndChallStage");  app->mWndChallStage->setVisible(false);
	app->mWndChallEnd   = fWnd("WndChallEnd");    app->mWndChallEnd->setVisible(false);

	app->mWndNetEnd = fWnd("WndNetEnd");  app->mWndNetEnd->setVisible(false);
	app->mWndTweak = fWnd("WndTweak");    app->mWndTweak->setVisible(false);
	app->mWndTweak->setPosition(0,40);
	

	//  for find defines
	Btn btn, bchk;  Cmb cmb;
	Slider* sl;  SV* sv;  Ck* ck;


	//  Tabs
	Tab tab,sub;
	fTabW("TabWndGame");    app->mWndTabsGame = tab;
	fTabW("TabWndReplays"); app->mWndTabsRpl = tab;
	fTabW("TabWndHelp");    app->mWndTabsHelp = tab;
	fTabW("TabWndOptions"); app->mWndTabsOpts = tab;

	//  get sub tabs
	vSubTabsGame.clear();
	for (size_t i=0; i < app->mWndTabsGame->getItemCount(); ++i)
	{	// todo: startsWith("SubTab")..
		sub = (Tab)app->mWndTabsGame->getItemAt(i)->findWidget(
			i==TAB_Champs ? "ChampType" : (i==TAB_Multi ? "tabsNet" : "tabPlayer") );
		vSubTabsGame.push_back(sub);  // 0 for not found
	}
	vSubTabsOpts.clear();
	for (size_t i=0; i < app->mWndTabsOpts->getItemCount(); ++i)
	{
		sub = (Tab)app->mWndTabsOpts->getItemAt(i)->findWidget(
			i==TABo_Input ? "InputTab" : "SubTab");
		vSubTabsOpts.push_back(sub);
	}

	if (pSet->inMenu > MNU_Single && pSet->inMenu <= MNU_Challenge)
		app->mWndTabsGame->setIndexSelected(TAB_Champs);

	app->mWndRpl = fWnd("RplWnd");


	///  Gui common init  ---
	gcom->InitMainMenu();
	gcom->GuiInitTooltip();
	gcom->GuiInitLang();

	gcom->GuiInitGraphics();
	gcom->InitGuiScreenRes();


	loadReadme = true;
	toggleGui(false);
	app->updMouse();
	gcom->bnQuit->setVisible(app->isFocGui);
	

	///  Sliders
	//------------------------------------------------------------------------
	    
	//  Hud view sizes  ----
	sv= &svSizeGaug;	sv->Init("SizeGaug",	&pSet->size_gauges,    0.1f, 0.3f,  1.f, 3,4);  sv->DefaultF(0.19f);  Sev(HudSize);
	sv= &svTypeGaug;	sv->Init("TypeGaug",	&pSet->gauges_type,    0, 5);  sv->DefaultI(5);  Sev(HudCreate);
	//sv= &svLayoutGaug;	sv->Init("LayoutGaug",	&pSet->gauges_layout,  0, 2);  sv->DefaultI(1);  Sev(HudCreate);

	sv= &svSizeMinimap;	sv->Init("SizeMinimap",	&pSet->size_minimap,   0.05f, 0.3f, 1.f, 3,4);  sv->DefaultF(0.165f);  Sev(HudSize);
	sv= &svZoomMinimap;	sv->Init("ZoomMinimap",	&pSet->zoom_minimap,   0.9f, 4.f,   1.f, 2,4);  sv->DefaultF(1.6f);    Sev(HudSize);
	sv= &svSizeArrow;	sv->Init("SizeArrow",   &pSet->size_arrow,     0.1f, 0.5f,  1.f, 3,4);  sv->DefaultF(0.26f);  Sev(SizeArrow);
	Slv(CountdownTime,  pSet->gui.pre_time / 0.5f /6.f);  sl->mfDefault = 4.f /6.f;


	//  graphs
	valGraphsType = fTxt("GraphsTypeVal");
	Cmb(cmb, "CmbGraphsType", comboGraphs);  cmbGraphs = cmb;
	if (cmb)
	{	cmb->removeAllItems();
		for (int i=0; i < Gh_ALL; ++i)
			cmb->addItem(csGraphNames[i]);
		cmb->setIndexSelected(pSet->graphs_type);
	}
	valGraphsType->setCaption(toStr(pSet->graphs_type));


	//  Options  ----
	sv= &svParticles;	sv->Init("Particles",	&pSet->particles_len, 0.f, 4.f, 2.f);  sv->DefaultF(1.5f);
	sv= &svTrails;		sv->Init("Trails",		&pSet->trails_len,    0.f, 4.f, 2.f);  sv->DefaultF(3.f);

	//  reflection
	sv= &svReflSkip;	sv->Init("ReflSkip",	&pSet->refl_skip,    0,1000, 2.f);  sv->DefaultI(0);
	sv= &svReflFaces;	sv->Init("ReflFaces",	&pSet->refl_faces,   1,6);  sv->DefaultI(1);
	sv= &svReflSize;
		for (int i=0; i < ciShadowSizesNum; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("ReflSize",	&pSet->refl_size,    0,ciShadowSizesNum-1);  sv->DefaultI(1.5f);

	sv= &svReflDist;	sv->Init("ReflDist",	&pSet->refl_dist,   20.f,1500.f, 2.f, 0,4, 1.f," m");
																	Sev(ReflDist);  sv->DefaultF(300.f);
	sv= &svReflMode;
		sv->strMap[0] = TR("#{ReflMode_static}");  sv->strMap[1] = TR("#{ReflMode_single}");
		sv->strMap[2] = TR("#{ReflMode_full}");
						sv->Init("ReflMode",	&pSet->refl_mode,   0,2);  Sev(ReflMode);  sv->DefaultI(1);

	//  Sound
	sv= &svVolMaster;	sv->Init("VolMaster",	&pSet->vol_master, 0.f, 1.6f);  sv->DefaultF(0.80f);  Sev(VolMaster);

	sv= &svVolEngine;	sv->Init("VolEngine",	&pSet->vol_engine, 0.f, 1.4f);  sv->DefaultF(0.56f);
	sv= &svVolTires;	sv->Init("VolTires",	&pSet->vol_tires,  0.f, 1.4f);  sv->DefaultF(0.80f);
	sv= &svVolSusp;		sv->Init("VolSusp",		&pSet->vol_susp,   0.f, 1.4f);  sv->DefaultF(0.714f);
	sv= &svVolEnv;		sv->Init("VolEnv",		&pSet->vol_env,    0.f, 1.4f);  sv->DefaultF(0.928f);

	sv= &svVolFlSplash;	sv->Init("VolFlSplash",	&pSet->vol_fl_splash, 0.f, 1.4f);  sv->DefaultF(0.80f);
	sv= &svVolFlCont;	sv->Init("VolFlCont",	&pSet->vol_fl_cont,   0.f, 1.4f);  sv->DefaultF(0.878f);
	sv= &svVolCarCrash;	sv->Init("VolCarCrash",	&pSet->vol_car_crash, 0.f, 1.4f);  sv->DefaultF(0.703f);
	sv= &svVolCarScrap;	sv->Init("VolCarScrap",	&pSet->vol_car_scrap, 0.f, 1.4f);  sv->DefaultF(1.00f);


	//  car color
	float f;  // temp
	sv= &svCarClrH;		sv->Init("CarClrH", &f, 0.f,1.f);  Sev(CarClr);
	sv= &svCarClrS;		sv->Init("CarClrS", &f, 0.f,1.f);  Sev(CarClr);
	sv= &svCarClrV;		sv->Init("CarClrV", &f, 0.f,1.f);  Sev(CarClr);
	sv= &svCarClrGloss;	sv->Init("CarClrGloss", &f, 0.f,1.f, 1.6f);  Sev(CarClr);
	sv= &svCarClrRefl;	sv->Init("CarClrRefl",  &f, 0.f,1.4f);  Sev(CarClr);
	UpdCarClrSld();


	///  Checks
	//------------------------------------------------------------------------
	ck= &ckReverse;		ck->Init("ReverseOn",	&pSet->gui.trackreverse);  Cev(Reverse);

	//  Options  ----
	ck= &ckParticles;	ck->Init("ParticlesOn", &pSet->particles);   Cev(ParTrl);
	ck= &ckTrails;		ck->Init("TrailsOn",    &pSet->trails);      Cev(ParTrl);

	//  Hud  ----
	ck= &ckDigits;		ck->Init("Digits",      &pSet->show_digits);   Cev(HudShow);
	ck= &ckGauges;		ck->Init("Gauges",      &pSet->show_gauges);   Cev(HudShow);

	ck= &ckArrow;		ck->Init("Arrow",       &pSet->check_arrow);   Cev(Arrow);
	ck= &ckBeam;		ck->Init("ChkBeam",     &pSet->check_beam);    Cev(Beam);

	//  minimap
	ck= &ckMinimap;		ck->Init("Minimap",     &pSet->trackmap);      Cev(Minimap);
	ck= &ckMiniZoom;	ck->Init("MiniZoom",    &pSet->mini_zoomed);   Cev(MiniUpd);
	ck= &ckMiniRot;		ck->Init("MiniRot",     &pSet->mini_rotated);
	ck= &ckMiniTer;		ck->Init("MiniTer",     &pSet->mini_terrain);  Cev(MiniUpd);
	ck= &ckMiniBorder;	ck->Init("MiniBorder",  &pSet->mini_border);   Cev(MiniUpd);

	//  camera
	ck= &ckCamInfo;		ck->Init("CamInfo",     &pSet->show_cam);   Cev(HudShow);
	ck= &ckCamTilt;		ck->Init("CamTilt",     &pSet->cam_tilt);
	ck= &ckCamLoop;		ck->Init("CamLoop",     &pSet->cam_loop_chng);


	//  times, opp
	ck= &ckTimes;		ck->Init("Times",       &pSet->show_times);      Cev(HudShow);
	ck= &ckOpponents;	ck->Init("Opponents",   &pSet->show_opponents);  Cev(HudShow);
	ck= &ckOppSort;		ck->Init("OppSort",     &pSet->opplist_sort);


	//  dbg,other
	ck= &ckFps;			ck->Init("Fps",			&pSet->show_fps);
	app->bckFps->setVisible(pSet->show_fps);
	ck= &ckWireframe;	ck->Init("Wireframe",   &app->mbWireFrame);  Cev(Wireframe);

	ck= &ckProfilerTxt;	ck->Init("ProfilerTxt",  &pSet->profilerTxt);
	ck= &ckBulletDebug;	ck->Init("BulletDebug",  &pSet->bltDebug);
	ck= &ckBltProfTxt;	ck->Init("BltProfTxt",   &pSet->bltProfilerTxt);

	ck= &ckCarDbgBars;	ck->Init("CarDbgBars",  &pSet->car_dbgbars);   Cev(HudShow);
	ck= &ckCarDbgTxt;	ck->Init("CarDbgTxt",   &pSet->car_dbgtxt);    Cev(HudShow);
	ck= &ckCarDbgSurf;	ck->Init("CarDbgSurf",  &pSet->car_dbgsurf);   Cev(HudShow);
	
	ck= &ckTireVis;		ck->Init("CarTireVis",  &pSet->car_tirevis);   Cev(HudCreate);
	ck= &ckGraphs;		ck->Init("Graphs",		&pSet->show_graphs);   Cev(Graphs);

	sv= &svDbgTxtClr;	sv->Init("DbgTxtClr",	&pSet->car_dbgtxtclr, 0, 1);
	sv= &svDbgTxtCnt;	sv->Init("DbgTxtCnt",	&pSet->car_dbgtxtcnt, 0, 8);


	//  car setup  todo: for each player ..
	Chk("CarABS",  chkAbs, pSet->abs[0]);  bchAbs = bchk;
	Chk("CarTCS",  chkTcs, pSet->tcs[0]);  bchTcs = bchk;
	
	ck= &ckCarGear;		ck->Init("CarGear",		&pSet->autoshift);  Cev(Gear);
	ck= &ckCarRear;		ck->Init("CarRear",		&pSet->autorear);   Cev(Gear);
	ck= &ckCarRearInv;	ck->Init("CarRearThrInv",&pSet->rear_inv);  Cev(Gear);

	TabPtr tTires = fTab("tabCarTires");  Tev(tTires, TireSet);
	
	sv= &svSSSEffect;		sv->Init("SSSEffect",		&f, 0.f, 1.f);
	sv= &svSSSVelFactor;	sv->Init("SSSVelFactor",	&f, 0.f, 2.f);
	sv= &svSteerRangeSurf;	sv->Init("SteerRangeSurf",	&f, 0.3f, 1.3f);
	sv= &svSteerRangeSim;	sv->Init("SteerRangeSim",	&f, 0.3f, 1.3f);
	Btn("SSSReset", btnSSSReset);  Btn("SteerReset", btnSteerReset);
	SldUpd_TireSet();


	//  game  ------------------------------------------------------------
	sv= &svDamageDec;	sv->Init("DamageDec",	&pSet->gui.damage_dec, 0.f, 100.f, 1.f, 0,1, 1.f, " %");
	ck= &ckVegetCollis;		ck->Init("VegetCollis",		&pSet->gui.collis_veget);
	ck= &ckCarCollis;		ck->Init("CarCollis",		&pSet->gui.collis_cars);
	ck= &ckRoadWCollis;		ck->Init("RoadWCollis",		&pSet->gui.collis_roadw);
	ck= &ckDynamicObjs;		ck->Init("DynamicObjects",	&pSet->gui.dyn_objects);

	Cmb(cmb, "CmbBoost", comboBoost);	cmbBoost = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{Never}"));		cmb->addItem(TR("#{FuelLap}"));
		cmb->addItem(TR("#{FuelTime}"));	cmb->addItem(TR("#{Always}"));
		cmb->setIndexSelected(pSet->gui.boost_type);

	Cmb(cmb, "CmbFlip", comboFlip);		cmbFlip = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{Never}"));		cmb->addItem(TR("#{FuelBoost}"));
		cmb->addItem(TR("#{Always}"));
		cmb->setIndexSelected(pSet->gui.flip_type);

	Cmb(cmb, "CmbDamage", comboDamage);	cmbDamage = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{None}"));		cmb->addItem(TR("#{Reduced}"));
		cmb->addItem(TR("#{Normal}"));
		cmb->setIndexSelected(pSet->gui.damage_type);

	Cmb(cmb, "CmbRewind", comboRewind);	cmbRewind = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{None}"));		cmb->addItem(TR("#{Always}"));
		//cmb->addItem(TR("#{FuelLap}"));		cmb->addItem(TR("#{FuelTime}"));
		cmb->setIndexSelected(pSet->gui.rewind_type);

	//  split
	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	
	ck= &ckSplitVert;	ck->Init("chkSplitVertically", &pSet->split_vertically);
	Chk("chkStartOrderRev", chkStartOrd, pSet->gui.start_order);
	valLocPlayers = fTxt("valLocPlayers");
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->gui.local_players));


	//  sim mode radio
	Btn("SimModeEasy", radSimEasy);  bRsimEasy = btn;  btn->setStateSelected(pSet->gui.sim_mode == "easy");
	Btn("SimModeNorm", radSimNorm);	 bRsimNorm = btn;  btn->setStateSelected(pSet->gui.sim_mode == "normal");

	//  kmh/mph radio
	Btn("kmh", radKmh);  bRkmh = btn;  bRkmh->setStateSelected(!pSet->show_mph);
	Btn("mph", radMph);	 bRmph = btn;  bRmph->setStateSelected( pSet->show_mph);


	//  startup
	ck= &ckStartInMain;	ck->Init("StartInMain", &pSet->startInMain);
	ck= &ckAutoStart;	ck->Init("AutoStart",   &pSet->autostart);
	ck= &ckEscQuits;	ck->Init("EscQuits",    &pSet->escquit);
	ck= &ckOgreDialog;	ck->Init("OgreDialog",  &pSet->ogre_dialog);
	ck= &ckMouseCapture;ck->Init("MouseCapture",&pSet->mouse_capture);

	ck= &ckBltLines;	ck->Init("BltLines",	&pSet->bltLines);
	ck= &ckShowPics;	ck->Init("ShowPictures",&pSet->loadingbackground);
	Chk("MultiThread", chkMultiThread, pSet->multi_thr > 0);
	ck= &ckDevKeys;		ck->Init("DevKeys",		&pSet->dev_keys);

	
	//  effects
	ck= &ckAllEffects;	ck->Init("AllEffects",	&pSet->all_effects);  Cev(AllEffects);
	ck= &ckBoostFOV;	ck->Init("BoostFOV",	&pSet->boost_fov);

	ck= &ckBloom;		ck->Init("Bloom",		&pSet->bloom);  Cev(EffUpd);
	sv= &svBloomInt;	sv->Init("BloomInt",	&pSet->bloom_int);   sv->DefaultF(0.13f);
	sv= &svBloomOrig;	sv->Init("BloomOrig",	&pSet->bloom_orig);	 sv->DefaultF(0.91f);
	
	ck= &ckBlur;		ck->Init("MotionBlur",	&pSet->blur);  Cev(EffUpdShd);
	sv= &svBlurIntens;	sv->Init("BlurIntens",	&pSet->blur_int);	sv->DefaultF(0.4f);

	ck= &ckSSAO;		ck->Init("SSAO",		&pSet->ssao);  Cev(EffUpdShd);
	ck= &ckSoftPar;		ck->Init("SoftParticles",&pSet->softparticles);  Cev(EffUpdShd);
	ck= &ckGodRays;		ck->Init("GodRays",		&pSet->godrays);  Cev(EffUpdShd);
	
	ck= &ckDoF;			ck->Init("DepthOfField",&pSet->dof);  Cev(EffUpdShd);
	sv= &svDofFocus;	sv->Init("DofFocus",	&pSet->dof_focus, 0.f, 2000.f, 2.f, 0,3);	sv->DefaultF(100.f);
	sv= &svDofFar;		sv->Init("DofFar",		&pSet->dof_far,   0.f, 2000.f, 2.f, 0,4);	sv->DefaultF(1000.f);

	//  hdr
	ck= &ckHDR;				ck->Init("HDR",				&pSet->hdr);  Cev(EffUpd);
	sv= &svHDRParam1;		sv->Init("HDRParam1",		&pSet->hdrParam1);  sv->DefaultF(0.62f);
	sv= &svHDRParam2;		sv->Init("HDRParam2",		&pSet->hdrParam2, 0.f, 1.f, 2.f);  sv->DefaultF(0.10f);
	sv= &svHDRParam3;		sv->Init("HDRParam3",		&pSet->hdrParam3);  sv->DefaultF(0.79f);
	sv= &svHDRAdaptScale;	sv->Init("HDRAdaptScale",	&pSet->hdrAdaptationScale, 0.f, 1.f, 2.f);  sv->DefaultF(0.10f);
	sv= &svHDRBloomInt;		sv->Init("HDRBloomInt",		&pSet->hdrBloomint);   sv->DefaultF(0.81f);
	sv= &svHDRBloomOrig;	sv->Init("HDRBloomOrig",	&pSet->hdrBloomorig);  sv->DefaultF(0.34f);
	sv= &svHDRVignRadius;	sv->Init("HDRVignRadius",	&pSet->vignRadius, 0.f, 10.f);  sv->DefaultF(2.85f);
	sv= &svHDRVignDark;		sv->Init("HDRVignDark",		&pSet->vignDarkness);  sv->DefaultF(0.34f);
	
	
	//  replays  ------------------------------------------------------------
	Btn("RplLoad",   btnRplLoad);    Btn("RplSave",   btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	ck= &ckRplAutoRec;		ck->Init("RplChkAutoRec",	&app->bRplRec);
	ck= &ckRplBestOnly;		ck->Init("RplChkBestOnly",	&pSet->rpl_bestonly);
	ck= &ckRplGhost;		ck->Init("RplChkGhost",		&pSet->rpl_ghost);
	ck= &ckRplParticles;	ck->Init("RplChkParticles",	&pSet->rpl_ghostpar);
		   
	ck= &ckRplRewind;		ck->Init("RplChkRewind",	&pSet->rpl_ghostrewind);
	ck= &ckRplGhostOther;	ck->Init("RplChkGhostOther",&pSet->rpl_ghostother);
	ck= &ckRplTrackGhost;	ck->Init("RplChkTrackGhost",&pSet->rpl_trackghost);
	Slv(RplNumViewports, (pSet->rpl_numViews-1) / 3.f);

	//  radios, filter
	ck= &ckRplGhosts;	ck->Init("RplBtnGhosts",  &pSet->rpl_listghosts);  Cev(RplGhosts);
	Btn("RplBtnAll", btnRplAll);  rbRplAll = btn;
	Btn("RplBtnCur", btnRplCur);  rbRplCur = btn;
	btn = pSet->rpl_listview == 0 ? rbRplAll : rbRplCur;
	if (btn)  btn->setStateSelected(true);

	
    if (app->mWndRpl)
	{	//  replay controls
		Btn("RplToStart", btnRplToStart);  Btn("RplToEnd", btnRplToEnd)
		Btn("RplPlay", btnRplPlay);  btRplPl = btn;
		btn = fBtn("RplBack");		btn->eventMouseButtonPressed += newDelegate(this, &CGui::btnRplBackDn);  btn->eventMouseButtonReleased += newDelegate(this, &CGui::btnRplBackUp);
		btn = fBtn("RplForward");	btn->eventMouseButtonPressed += newDelegate(this, &CGui::btnRplFwdDn);   btn->eventMouseButtonReleased += newDelegate(this, &CGui::btnRplFwdUp);
		
		//  info
		slRplPos = (Slider*)app->mWndRpl->findWidget("RplSlider");
		if (slRplPos)  slRplPos->eventValueChanged += newDelegate(this, &CGui::slRplPosEv);

		valRplPerc = fTxt("RplPercent");
    	valRplCur = fTxt("RplTimeCur");
    	valRplLen = fTxt("RplTimeLen");
	}
	//  text desc, stats
	valRplName = fTxt("RplName");  valRplName2 = fTxt("RplName2");
	valRplInfo = fTxt("RplInfo");  valRplInfo2 = fTxt("RplInfo2");
	edRplName = fEd("RplNameEdit");
	//edRplDesc = fEd("RplDesc");

	rplList = fLi("RplList");
	Lev(rplList, RplChng);
	updReplaysList();


	///  Car
	//------------------------------------------------------------
	const int clrBtn = 30;
	Real hsv[clrBtn][5] = {  // color buttons  hue,sat,val, gloss,refl
	{0.05,0.64,0.27, 0.10,0.9}, {0.00,0.97,0.90, 0.3, 1.2},  // cherry, red
	{0.91,1.00,1.00, 0.5, 1.0}, {0.86,1.00,0.97, 0.8, 0.6},  // orange, yellow
	{0.75,0.95,0.90, 1.0, 0.4}, {0.70,1.00,0.70, 0.03,1.3}, // lime, green
	{0.54,0.88,0.60, 0.7, 0.85},{0.51,0.90,0.50, 0.1, 0.7},  // cyan
	{0.41,0.34,0.30, 0.01,0.3}, {0.43,0.58,0.23, 0.1, 1.0},  // dark-cyan  //{0.45,0.54,0.37, 0.5,1.0},
	{0.37,0.78,0.21, 0.34,0.5}, {0.35,0.70,0.40, 0.5, 1.0}, {0.38,0.97,0.52, 0.5, 1.0},  // dark-blue
	{0.44,0.90,0.71, 1.0, 1.1}, {0.47,0.80,0.80, 0.2, 0.9}, // sky-blue
	{0.50,0.33,0.90, 0.9, 1.0}, {0.42,0.20,0.94, 0.5, 0.4}, // sky-white
	{0.63,0.21,0.62, 0.1, 1.2}, {0.80,0.52,0.32, 0.1, 0.6}, {0.62,0.74,0.12, 0.8, 0.7},  // olive-
	{0.28,0.00,0.12, 0.09,0.0}, {0.28,0.00,0.07, 0.14,0.84},  // black
	{0.83,0.00,0.20, 0.0, 0.8}, {0.41,0.00,0.86, 0.15,0.37}, // silver,white
	{0.83,0.31,0.31, 0.0, 0.6}, {0.91,0.40,0.37, 0.0, 1.0}, {0.20,0.40,0.37, 0.05,1.0},  // orng-white-
	{0.24,0.90,0.26, 0.04,0.8}, {0.28,0.57,0.17, 0.3, 1.0}, {0.27,0.38,0.23, 0.03,0.6},  // dark violet
	};
	for (int i=0; i < clrBtn; ++i)
	{
		Img img = fImg("carClr"+toStr(i));
		Real h = hsv[i][0], s = hsv[i][1], v = hsv[i][2], g = hsv[i][3], r = hsv[i][4];
		ColourValue c;  c.setHSB(1.f-h, s, v);
		img->setColour(Colour(c.r,c.g,c.b));
		img->eventMouseButtonClick += newDelegate(this, &CGui::imgBtnCarClr);
		img->setUserString("s", toStr(s));  img->setUserString("h", toStr(h));
		img->setUserString("v", toStr(v));  img->setUserString("g", toStr(g));  img->setUserString("r", toStr(r));
	}
	Btn("CarClrRandom", btnCarClrRandom);
	sv= &svNumLaps;  sv->Init("NumLaps",  &pSet->gui.num_laps, 1,20);

	txCarStatsTxt = fTxt("CarStatsTxt");
	txCarStatsVals = fTxt("CarStatsVals");

    txCarSpeed = fTxt("CarSpeed");
    txCarType = fTxt("CarType");

    txCarAuthor = fTxt("CarAuthor");
    txTrackAuthor = fTxt("TrackAuthor");
	
	TabPtr tPlr = fTab("tabPlayer");  Tev(tPlr, Player);
	
	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	ck= &ckSplitVert;	ck->Init("chkSplitVertically",  &pSet->split_vertically);


	///  Multiplayer
	//------------------------------------------------------------------------
	tabsNet = fTab("tabsNet");
		//TabItem* t1 = tabsNet->getItemAt(0);
		//t1->setEnabled(0);
	//int num = tabsNet ? tabsNet->getItemCount() : 0;
	//tabsNet->setIndexSelected( (tabsNet->getIndexSelected() - 1 + num) % num );
	
	//  server, games
	listServers = fMli("MListServers");  int c=0;
	Mli ml = listServers;
		ml->addColumn("#C0FFC0"+TR("#{Game name}"), 160);  ++c;
		ml->addColumn("#50FF50"+TR("#{Track}"), 120);  ++c;
		ml->addColumn("#80FFC0"+TR("#{Laps}"), 60);  ++c;
		ml->addColumn("#FFFF00"+TR("#{Players}"), 60);  ++c;
		ml->addColumn("#80FFFF"+TR("#{Collis.}"), 70);  ++c;
		ml->addColumn("#A0D0FF"+TR("#{Simulation}"), 80);  ++c;
		ml->addColumn("#A0D0FF"+TR("#{Boost}"), 90);  ++c;
		ml->addColumn("#FF6060"+TR("#{Locked}"), 60);  iColLock = c;  ++c;
		ml->addColumn("#FF9000"+TR("#{NetHost}"), 140);  iColHost = c;  ++c;
		ml->addColumn("#FFB000"+TR("#{NetPort}"), 80);  iColPort = c;  ++c;

	Btn("btnNetRefresh",evBtnNetRefresh); btnNetRefresh = btn;
	Btn("btnNetJoin",   evBtnNetJoin);    btnNetJoin = btn;
	Btn("btnNetCreate", evBtnNetCreate);  btnNetCreate = btn;
	Btn("btnNetDirect", evBtnNetDirect);  btnNetDirect = btn;

	//  game, players
	Edt(edNetGameName, "edNetGameName", evEdNetGameName);  edNetGameName->setCaption(pSet->netGameName);
	valNetPassword = fTxt("valNetPassword");
	Edt(edNetPassword, "edNetPassword", evEdNetPassword);

	listPlayers = fMli("MListPlayers");
	ml = listPlayers;
		ml->addColumn("#80C0FF"+TR("#{Player}"), 140);
		ml->addColumn("#F08080"+TR("#{Car}"), 60);
		ml->addColumn("#C0C060"+TR("#{Peers}"), 60);
		ml->addColumn("#60F0F0"+TR("#{Ping}"), 60);
		ml->addColumn("#40F040"+TR("#{NetReady}"), 60);

	Btn("btnNetReady", evBtnNetReady);  btnNetReady = btn;
	Btn("btnNetLeave", evBtnNetLeave);	btnNetLeave = btn;

	//  panels to hide tabs
	panNetServer = fWP("panelNetServer");  panNetServer->setVisible(false);
	panNetGame = fWP("panelNetGame");      panNetGame->setVisible(true);

    //  chat
    edNetChat = fEd("edNetChat");  // chat area
    edNetChatMsg = fEd("edNetChatMsg");  // user text
    //  track,game text
    valNetGameInfo = fTxt("valNetGameInfo");

	//  settings
	Edt(edNetNick,		"edNetNick",		evEdNetNick);		edNetNick->setCaption(		pSet->nickname);
	Edt(edNetServerIP,	"edNetServerIP",	evEdNetServerIP);	edNetServerIP->setCaption(	pSet->master_server_address);
	Edt(edNetServerPort,"edNetServerPort",	evEdNetServerPort);	edNetServerPort->setCaption(toStr(pSet->master_server_port));
	Edt(edNetLocalPort,	"edNetLocalPort",	evEdNetLocalPort);	edNetLocalPort->setCaption(	toStr(pSet->local_port));

	
	//  quick help text  ----
	Ed edHelp = fEd("QuickHelpText");
	String s = TR("#{QuickHelpText}");
	s = StringUtil::replaceAll(s, "@", "\n");
	edHelp->setCaption(s);

	//  user dir
    Ed edUserDir = fEd("EdUserDir");
	edUserDir->setCaption(PATHMANAGER::UserConfigDir());

	
	///  tweak  ----
	for (int i=0; i < ciEdCar; ++i)
		edCar[i] = fEd("EdCar"+toStr(i));
	edTweakCol = fEd("TweakEditCol");
	edPerfTest = fEd("TweakPerfTest");
	tabEdCar = fTab("TabEdCar");  Tev(tabEdCar, CarEdChng);  tabEdCar->setIndexSelected(pSet->car_ed_tab);

	tabTweak = fTab("TabTweak");
	txtTweakPath = fTxt("TweakPath");
	txtTweakPathCol = fTxt("TweakPathCol");
	txtTweakTire = fTxt("TweakTireSaved");

	Btn("TweakCarSave", btnTweakCarSave);  Btn("TweakTireSave", btnTweakTireSave);
	Btn("TweakColSave", btnTweakColSave);
	Cmb(cmbTweakTireSet,"TweakTireSet",CmbTweakTireSet);
	cmbTweakTireSet->eventEditTextChange += newDelegate(this, &CGui::CmbEdTweakTireSet);


	///  input tab  -------
	InitInputGui();
		
	
	///  cars list
    //------------------------------------------------------------------------
	Tbi carTab = fTbi("TabCar");
	Mli2 li = carTab->createWidget<MultiList2>("MultiListBox",16,48,200,110, Align::Left | Align::VStretch);
	li->setColour(Colour(0.7,0.85,1.0));
	li->removeAllColumns();  int n=0;
	li->addColumn("#FF8888"+TR("#{Car}"), colCar[n++]);
	li->addColumn("#FFC080"+TR("#{CarSpeed}"), colCar[n++]);
	li->addColumn("#B0B8C0"+TR("#{CarYear}"), colCar[n++]);
	li->addColumn("#C0C0E0"+TR("#{CarType}"), colCar[n++]);
	li->addColumn(" ", colCar[n++]);
	carList = li;

	FillCarList();  //once

	li->mSortColumnIndex = pSet->cars_sort;
	li->mSortUp = pSet->cars_sortup;
	Lev(carList, CarChng);

   	CarListUpd(false);  //upd

    sListCar = pSet->gui.car[0];
    imgCar = fImg("CarImg");
    carDesc = fEd("CarDesc");
    listCarChng(carList,0);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------

	gcom->trkDesc[0] = fEd("TrackDesc");
	gcom->sListTrack = pSet->gui.track;

    gcom->GuiInitTrack();

	Ed ed;
	Edt(ed,"RplFind",edRplFind);

	//  netw
	Tbi trkTab = fTbi("TabTrack");
	trkTab->setColour(Colour(0.8f,0.96f,1.f));
	const IntCoord& tc = trkTab->getCoord();

	WP wp = trkTab->createWidget<Widget>(
		"PanelSkin", 0,0,tc.width*0.66f,tc.height, Align::Default/*, "Popup", "panelNetTrack"*/);
	wp->setColour(Colour(0.8f,0.96f,1.f));
	wp->setAlpha(0.8f);  wp->setVisible(false);
	panNetTrack = wp;
	//<UserString key="RelativeTo" value="OptionsWnd"/>

    //  new game
    for (int i=1; i<=3; ++i)
    {	Btn("NewGame"+toStr(i), btnNewGame);  if (i==2)  btNewGameCar = btn;  }

	//  championships
	//------------------------------------------------------------------------

	//  track stats 2nd set
	gcom->trkDesc[1] = fEd("TrackDesc2");
    valTrkNet = fTxt("TrackText");

	//  preview images
	gcom->imgPrv[1] = fImg("TrackImg2");   gcom->imgPrv[1]->setImageTexture("PrvView");
	gcom->imgTer[1] = fImg("TrkTerImg2");  gcom->imgTer[1]->setImageTexture("PrvTer");
	gcom->imgMini[1] = fImg("TrackMap2");  gcom->imgMini[1]->setImageTexture("PrvRoad");

	//  track stats text
	int i;
	for (i=0; i < gcom->StTrk; ++i)   gcom->stTrk[1][i] =  fTxt("2iv"+toStr(i));
	for (i=0; i < gcom->InfTrk; ++i)  gcom->infTrk[1][i] = fTxt("2ti"+toStr(i));

	edChInfo = fEd("ChampInfo");
	if (edChInfo)  edChInfo->setVisible(pSet->champ_info);
	Btn("btnChampInfo",btnChampInfo);

	panCh = fWP("panCh");
	txtCh = fTxt("txtChDetail");
	valCh = fTxt("valChDetail");
	for (int i=0; i<3; ++i) {  String s = toStr(i);
		txtChP[i] = fTxt("txtChP"+s);
		valChP[i] = fTxt("valChP"+s);  }
	edChDesc = fEd("ChampDescr");


	//  Champs list  -------------
	Tbi trktab = fTbi("TabChamps");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	Lev(li, ChampChng);  li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colCh[c++]);
	li->addColumn(TR("#40F040#{Name}"), colCh[c++]);		li->addColumn(TR("#F0F040#{Difficulty}"), colCh[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colCh[c++]);		li->addColumn(TR("#80E0FF#{Time} #{TimeMS}"), colCh[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colCh[c++]);	li->addColumn(TR("#F0E0F0#{Score}"), colCh[c++]);
	li->addColumn(" ", colCh[c++]);
	liChamps = li;

	//  Challs list  -------------
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	Lev(li, ChallChng);  li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colChL[c++]);
	li->addColumn(TR("#60F060#{Name}"), colChL[c++]);		li->addColumn(TR("#F0D040#{Difficulty}"), colChL[c++]);
	li->addColumn(TR("#F09090#{Cars}"), colChL[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colChL[c++]);		li->addColumn(TR("#80E0FF#{Time} m"), colChL[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colChL[c++]);
	li->addColumn(TR("#F0F8FF#{Prize}"), colChL[c++]);		li->addColumn(TR("#F0D0F0#{Score}"), colChL[c++]);
	li->addColumn(" ", colChL[c++]);
	liChalls = li;

	//  Stages list  -------------
	trktab = (TabItem*)app->mWndGame->findWidget("TabStages");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.7,0.73,0.76));
	Lev(li, StageChng);  li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colSt[c++]);
	li->addColumn(TR("#50F050#{Track}"), colSt[c++]);		li->addColumn(TR("#80FF80#{Scenery}"), colSt[c++]);
	li->addColumn(TR("#F0F040#{Difficulty}"), colSt[c++]);	li->addColumn(TR("#60E0A0#{Laps}"), colSt[c++]);
	li->addColumn(TR("#80E0FF#{Time} #{TimeMS}"), colSt[c++]);	li->addColumn(TR("#F0E0F0#{Score}"), colSt[c++]);
	li->addColumn(" ", colSt[c++]);
	liStages = li;

	updChampListDim();
	ChampsListUpdate();  listChampChng(liChamps, liChamps->getIndexSelected());
	ChallsListUpdate();  listChallChng(liChalls, liChalls->getIndexSelected());


	//  tabs
	tabTut   = fTab("TutType");    Tev(tabTut,   TutType);    tabTut->setIndexSelected(pSet->tut_type);
	tabChamp = fTab("ChampType");  Tev(tabChamp, ChampType);  tabChamp->setIndexSelected(pSet->champ_type);
	tabChall = fTab("ChallType");  Tev(tabChall, ChallType);  tabChall->setIndexSelected(pSet->chall_type);
	imgTut   = fImg("imgTut");
	imgChamp = fImg("imgChamp");
	imgChall = fImg("imgChall");

	Btn("btnTutStart",  btnChampStart);  btStTut = btn;
	Btn("btnChampStart",btnChampStart);  btStChamp = btn;
	Btn("btnChallStart",btnChallStart);  btStChall = btn;
	Btn("btnChRestart", btnChRestart);   btChRestart = btn;


	//  ch other
	ck= &ckChampRev;	ck->Init("ChampRev",    &pSet->gui.champ_rev);   Cev(ChampRev);
	
	Btn("btnChampStageBack", btnChampStageBack);
	Btn("btnChampStageStart",btnChampStageStart);  btChampStage = btn;
	Btn("btnChampEndClose",  btnChampEndClose);

	Btn("btnChallStageBack", btnChallStageBack);
	Btn("btnChallStageStart",btnChallStageStart);  btChallStage = btn;
	Btn("btnChallEndClose",  btnChallEndClose);

	Btn("btnStageNext", btnStageNext);
	Btn("btnStagePrev", btnStagePrev);
    valStageNum = fTxt("StageNum");

	edChampStage = fEd("ChampStageText");  edChampEnd = fEd("ChampEndText");
	edChallStage = fEd("ChallStageText");  edChallEnd = fEd("ChallEndText");
	//  stage prv
	imgChampStage = fImg("ChampStageImg");  imgChampStage->setImageTexture("PrvStCh");
	imgChallStage = fImg("ChallStageImg");	imgChallStage->setImageTexture("PrvStCh");
	
	imgChampEndCup = fImg("ChampEndImgCup"); txChampEndF = fTxt("ChampEndFinished");
	imgChallFail = fImg("ChallEndImgFail");  txChallEndF = fTxt("ChallEndFinished");
	imgChallCup  = fImg("ChallEndImgCup");   txChallEndC = fTxt("ChallEndCongrats");

	UpdChampTabVis();


	//  netw end list  ------
	Btn("btnNetEndClose", btnNetEndClose);

	li = app->mWndNetEnd->createWidget<MultiList2>("MultiListBox",4,42,632,360, Align::Left | Align::VStretch);
	li->setInheritsAlpha(false);  li->setColour(Colour(0.8,0.9,1,1));
	li->removeAllColumns();
	li->addColumn("", 40);  //N
	li->addColumn(TR("#{TBPlace}"), 60);	li->addColumn(TR("#{NetNickname}"), 180);
	li->addColumn(TR("#{TBTime}"), 120);	li->addColumn(TR("#{TBBest}"), 120);
	li->addColumn(TR("#{TBLap}"), 60);
	liNetEnd = li;
	

	bGI = true;  // gui inited, gui events can now save vals
	
	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + fToStr(dt,0,3) + " ms");
}
