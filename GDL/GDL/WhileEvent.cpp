/** \file
 *  Game Develop
 *  2008-2011 Florian Rival (Florian.Rival@gmail.com)
 */

#include "WhileEvent.h"
#include "GDL/tinyxml.h"
#include "GDL/RuntimeScene.h"
#include "GDL/OpenSaveGame.h"
#include "GDL/EventsCodeGenerator.h"
#include "GDL/ExpressionsCodeGeneration.h"
#include "GDL/EventsCodeGenerationContext.h"

#if defined(GD_IDE_ONLY)
#include "GDL/EventsRenderingHelper.h"
#include "GDL/ExtensionsManager.h"
#include "GDL/EventsEditorItemsAreas.h"
#include "GDL/EventsEditorSelection.h"
#endif

std::string WhileEvent::GenerateEventCode(const Game & game, const Scene & scene, EventsCodeGenerationContext & parentContext)
{
    std::string outputCode;

    //Context is "reset" each time the event is repeated ( i.e. objects are picked again )
    EventsCodeGenerationContext context;
    context.InheritsFrom(parentContext);

    //Prepare codes
    std::string whileConditionsStr = EventsCodeGenerator::GenerateConditionsListCode(game, scene, whileConditions, parentContext);
    std::string whileIfPredicat = "true"; for (unsigned int i = 0;i<whileConditions.size();++i) whileIfPredicat += " && condition"+ToString(i)+"IsTrue";
    std::string conditionsCode = EventsCodeGenerator::GenerateConditionsListCode(game, scene, conditions, context);
    std::string actionsCode = EventsCodeGenerator::GenerateActionsListCode(game, scene, actions, context);
    std::string ifPredicat = "true"; for (unsigned int i = 0;i<conditions.size();++i) ifPredicat += " && condition"+ToString(i)+"IsTrue";

    //Write final code
    outputCode += "bool stopDoWhile = false;";
    outputCode += "do";
    outputCode += "{\n";
    outputCode += context.GenerateObjectsDeclarationCode();
    outputCode +=  whileConditionsStr;
    outputCode += "if ("+whileIfPredicat+")\n";
    outputCode += "{\n";
    outputCode += conditionsCode;
    outputCode += "if (" +ifPredicat+ ")\n";
    outputCode += "{\n";
    outputCode += actionsCode;
    outputCode += "\n{ //Subevents: \n";
    outputCode += EventsCodeGenerator::GenerateEventsListCode(game, scene, events, context);
    outputCode += "} //Subevents end.\n";
    outputCode += "}\n";
    outputCode += "} else stopDoWhile = true; \n";

    outputCode += "} while ( !stopDoWhile );\n";

    return outputCode;
}

vector < vector<Instruction>* > WhileEvent::GetAllConditionsVectors()
{
    vector < vector<Instruction>* > allConditions;
    allConditions.push_back(&whileConditions);
    allConditions.push_back(&conditions);

    return allConditions;
}

vector < vector<Instruction>* > WhileEvent::GetAllActionsVectors()
{
    vector < vector<Instruction>* > allActions;
    allActions.push_back(&actions);

    return allActions;
}
#if defined(GD_IDE_ONLY)
void WhileEvent::SaveToXml(TiXmlElement * eventElem) const
{
    //Save "While conditions"
    TiXmlElement * whileConditionsElem = new TiXmlElement( "WhileConditions" );
    eventElem->LinkEndChild( whileConditionsElem );
    OpenSaveGame::SaveConditions(whileConditions, whileConditionsElem);

    //Les conditions
    TiXmlElement * conditionsElem = new TiXmlElement( "Conditions" );
    eventElem->LinkEndChild( conditionsElem );
    OpenSaveGame::SaveConditions(conditions, conditionsElem);

    //Les actions
    TiXmlElement * actionsElem = new TiXmlElement( "Actions" );
    eventElem->LinkEndChild( actionsElem );
    OpenSaveGame::SaveActions(actions, actionsElem);

    //Sous �v�nements
    if ( !GetSubEvents().empty() )
    {
        TiXmlElement * subeventsElem;
        subeventsElem = new TiXmlElement( "Events" );
        eventElem->LinkEndChild( subeventsElem );

        OpenSaveGame::SaveEvents(events, subeventsElem);
    }
}
#endif

void WhileEvent::LoadFromXml(const TiXmlElement * eventElem)
{
    if ( eventElem->FirstChildElement( "WhileConditions" ) != NULL )
        OpenSaveGame::OpenConditions(whileConditions, eventElem->FirstChildElement( "WhileConditions" ));
    else
        cout << "Aucune informations sur les while conditions d'un �v�nement While";

    //Conditions
    if ( eventElem->FirstChildElement( "Conditions" ) != NULL )
        OpenSaveGame::OpenConditions(conditions, eventElem->FirstChildElement( "Conditions" ));
    else
        cout << "Aucune informations sur les conditions d'un �v�nement";

    //Actions
    if ( eventElem->FirstChildElement( "Actions" ) != NULL )
        OpenSaveGame::OpenActions(actions, eventElem->FirstChildElement( "Actions" ));
    else
        cout << "Aucune informations sur les actions d'un �v�nement";

    //Subevents
    if ( eventElem->FirstChildElement( "Events" ) != NULL )
        OpenSaveGame::OpenEvents(events, eventElem->FirstChildElement( "Events" ));
}

#if defined(GD_IDE_ONLY)
/**
 * Render the event in the bitmap
 */
void WhileEvent::Render(wxDC & dc, int x, int y, unsigned int width, EventsEditorItemsAreas & areas, EventsEditorSelection & selection)
{
    EventsRenderingHelper * renderingHelper = EventsRenderingHelper::GetInstance();
    int border = renderingHelper->instructionsListBorder;
    const int repeatHeight = 20;

    //Draw event rectangle
    wxRect rect(x, y, width, GetRenderedHeight(width));
    renderingHelper->DrawNiceRectangle(dc, rect);

    //While text
    dc.SetFont( renderingHelper->GetBoldFont() );
    dc.DrawText( _("Tant que :"), x+5, y+5 );

    //Draw "while conditions"
    int whileConditionsHeight = renderingHelper->DrawConditionsList(whileConditions, dc, x+80+border, y+border, width-80-border*2, this, areas, selection);

    dc.SetFont( renderingHelper->GetBoldFont() );
    dc.DrawText( _("R�p�ter :"), x+2, y+whileConditionsHeight);
    whileConditionsHeight += repeatHeight;

    renderingHelper->DrawConditionsList(conditions, dc,
                                        x+border,
                                        y+whileConditionsHeight+border+border*2,
                                        renderingHelper->GetConditionsColumnWidth()-border*2, this, areas, selection);
    renderingHelper->DrawActionsList(actions, dc,
                                     x+renderingHelper->GetConditionsColumnWidth()+border,
                                     y+whileConditionsHeight+border+border*2,
                                     width-renderingHelper->GetConditionsColumnWidth()-border*2, this, areas, selection);
}

unsigned int WhileEvent::GetRenderedHeight(unsigned int width) const
{
    if ( eventHeightNeedUpdate )
    {
        EventsRenderingHelper * renderingHelper = EventsRenderingHelper::GetInstance();
        int border = renderingHelper->instructionsListBorder;
        const int repeatHeight = 20;

        //Get maximum height needed
        int whileConditionsHeight = renderingHelper->GetRenderedConditionsListHeight(whileConditions, width-80-border*2);
        int conditionsHeight = renderingHelper->GetRenderedConditionsListHeight(conditions, renderingHelper->GetConditionsColumnWidth()-border*2);
        int actionsHeight = renderingHelper->GetRenderedActionsListHeight(actions, width-renderingHelper->GetConditionsColumnWidth()-border*2);

        renderedHeight = (( conditionsHeight > actionsHeight ? conditionsHeight : actionsHeight ) + whileConditionsHeight + repeatHeight)+border*2+border*2;
        eventHeightNeedUpdate = false;
    }

    return renderedHeight;
}

void WhileEvent::EditEvent(wxWindow* parent_, Game & game_, Scene & scene_, MainEditorCommand & mainEditorCommand_)
{
}
#endif

/**
 * Initialize from another WhileEvent.
 * Used by copy ctor and assignement operator
 */
void WhileEvent::Init(const WhileEvent & event)
{
    events = CloneVectorOfEvents(event.events);

    whileConditions = event.whileConditions;
    conditions = event.conditions;
    actions = event.actions;
}

/**
 * Custom copy operator
 */
WhileEvent::WhileEvent(const WhileEvent & event) :
BaseEvent(event)
{
    Init(event);
}

/**
 * Custom assignement operator
 */
WhileEvent& WhileEvent::operator=(const WhileEvent & event)
{
    if ( this != &event )
    {
        BaseEvent::operator=(event);
        Init(event);
    }

    return *this;
}
