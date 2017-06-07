#include "justina_tools/JustinaRepresentation.h"

ros::Publisher * JustinaRepresentation::command_runCLIPS;
ros::Publisher * JustinaRepresentation::command_resetCLIPS;
ros::Publisher * JustinaRepresentation::command_factCLIPS;
ros::Publisher * JustinaRepresentation::command_ruleCLIPS;
ros::Publisher * JustinaRepresentation::command_agendaCLIPS;
ros::Publisher * JustinaRepresentation::command_sendCLIPS;
ros::Publisher * JustinaRepresentation::command_loadCLIPS;
ros::Publisher * JustinaRepresentation::command_sendAndRunCLIPS;
ros::Publisher * JustinaRepresentation::command_response;
ros::ServiceClient * JustinaRepresentation::cliSpechInterpretation;
ros::ServiceClient * JustinaRepresentation::cliStringInterpretation;
ros::Subscriber * JustinaRepresentation::subQueryResult;

bool JustinaRepresentation::queryResultReceive = false;
std::string JustinaRepresentation::queryResult = "";

JustinaRepresentation::~JustinaRepresentation(){
    delete command_runCLIPS;
    delete command_resetCLIPS;
    delete command_factCLIPS;
    delete command_ruleCLIPS;
    delete command_agendaCLIPS;
    delete command_sendCLIPS;
    delete command_loadCLIPS;
    delete command_sendAndRunCLIPS;
    delete cliSpechInterpretation;
    delete cliStringInterpretation;
    delete subQueryResult;
    delete command_response;
}

void JustinaRepresentation::setNodeHandle(ros::NodeHandle * nh) {
    command_runCLIPS = new ros::Publisher(nh->advertise<std_msgs::Bool>("/planning_clips/command_runCLIPS", 1));
    command_resetCLIPS = new ros::Publisher(nh->advertise<std_msgs::Bool>("/planning_clips/command_resetCLIPS", 1));
    command_factCLIPS = new ros::Publisher(nh->advertise<std_msgs::Bool>("/planning_clips/command_factCLIPS", 1));
    command_ruleCLIPS = new ros::Publisher(nh->advertise<std_msgs::Bool>("/planning_clips/command_ruleCLIPS", 1));
    command_agendaCLIPS = new ros::Publisher(nh->advertise<std_msgs::Bool>("/planning_clips/command_agendaCLIPS", 1));
    command_sendCLIPS = new ros::Publisher(nh->advertise<std_msgs::String>("/planning_clips/command_sendCLIPS", 1));
    command_loadCLIPS = new ros::Publisher(nh->advertise<std_msgs::String>("/planning_clips/command_loadCLIPS", 1));
    command_sendAndRunCLIPS = new ros::Publisher(nh->advertise<std_msgs::String>("/planning_clips/command_sendAndRunCLIPS", 1));
    cliSpechInterpretation = new ros::ServiceClient(nh->serviceClient<knowledge_msgs::planning_cmd>("/planning_clips/spr_interpreter"));
    cliStringInterpretation = new ros::ServiceClient(nh->serviceClient<knowledge_msgs::planning_cmd>("/planning_clips/str_interpreter"));
    subQueryResult = new ros::Subscriber(nh->subscribe("/planning_clips/cmd_query_result", 1, callbackQueryResult));
    command_response = new ros::Publisher(nh->advertise<knowledge_msgs::PlanningCmdClips>("/planning_clips/command_response", 1));
}

void JustinaRepresentation::callbackQueryResult(const knowledge_msgs::PlanningCmdClips &planningCmdClips){
    std::cout << "JustinaRepresentation.->Answer a question." << planningCmdClips.params;
    knowledge_msgs::PlanningCmdClips cmd_response;
    queryResultReceive = true;
    queryResult = planningCmdClips.params;
    cmd_response.name = planningCmdClips.name;
    cmd_response.params = planningCmdClips.params;
    cmd_response.id = planningCmdClips.id;
    cmd_response.successful= 1;
    command_response->publish(cmd_response);
}

void JustinaRepresentation::runCLIPS(bool enable){
    std_msgs::Bool msg;
    msg.data = enable;
    command_runCLIPS->publish(msg);
}

void JustinaRepresentation::resetCLIPS(bool enable){
    std_msgs::Bool msg;
    msg.data = enable;
    command_resetCLIPS->publish(msg);
}

void JustinaRepresentation::factCLIPS(bool enable){
    std_msgs::Bool msg;
    msg.data = enable;
    command_factCLIPS->publish(msg);
}

void JustinaRepresentation::ruleCLIPS(bool enable){
    std_msgs::Bool msg;
    msg.data = enable;
    command_ruleCLIPS->publish(msg);
}

void JustinaRepresentation::agendaCLIPS(bool enable){
    std_msgs::Bool msg;
    msg.data = enable;
    command_agendaCLIPS->publish(msg);
}

void JustinaRepresentation::sendCLIPS(std::string command){
    std_msgs::String msg;
    msg.data = command;
    command_sendCLIPS->publish(msg);
}

void JustinaRepresentation::loadCLIPS(std::string file)
{
    std_msgs::String msg;
    msg.data = file;
    command_loadCLIPS->publish(msg);
}

void JustinaRepresentation::sendAndRunCLIPS(std::string command){
    std_msgs::String msg;
    msg.data = command;
    command_sendAndRunCLIPS->publish(msg);
}

void JustinaRepresentation::getLocations(std::string path, std::map<std::string, std::vector<std::string> > &locations)
{
    std::cout << "Ltm.->Loading known locations from " << path << std::endl;
    std::vector<std::string> lines;
    std::ifstream file(path.c_str());
    std::string tempStr;
    while (std::getline(file, tempStr))
        lines.push_back(tempStr);

    //Extraction of lines without comments
    for (size_t i = 0; i < lines.size(); i++) {
        size_t idx = lines[i].find("//");
        if (idx != std::string::npos)
            lines[i] = lines[i].substr(0, idx);
    }

    locations.clear();
    float locX, locY, locAngle;
    bool parseSuccess;
    for (size_t i = 0; i < lines.size(); i++) {
        //std::cout << "Ltm.->Parsing line: " << lines[i] << std::endl;
        std::vector<std::string> parts;
        std::vector<std::string> loc;
        boost::split(parts, lines[i], boost::is_any_of(" ,\t"),
                boost::token_compress_on);
        if (parts.size() < 3)
            continue;
        //std::cout << "Ltm.->Parsing splitted line: " << lines[i] << std::endl;
        parseSuccess = true;
        loc.push_back(parts[0]);
        loc.push_back(parts[2]);
        loc.push_back(parts[3]);

        if (parseSuccess) {
            //std::cout << "LOCATIONS" << parts[0] << std::endl;
            locations[parts[1]] = loc;
        }
    }
    std::cout << "Ltm.->Total number of known locations: " << locations.size() << std::endl;
    for (std::map<std::string, std::vector<std::string> >::iterator it = locations.begin(); it != locations.end(); it++) {
        std::cout << "Ltm.->Location " << it->first << " " << it->second[0] << " " << it->second[1];
        if (it->second.size() > 2)
            std::cout << " " << it->second[2];
        std::cout << std::endl;
    }
    if (locations.size() < 1)
        std::cout << "Ltm.->WARNING: Cannot load known locations from file: "
            << path << ". There are no known locations." << std::endl;

}

void JustinaRepresentation::getObjects(std::string path, std::map<std::string, std::vector<std::string> > &objects)
{
    std::cout << "Ltm.->Loading known locations from " << path << std::endl;
    std::vector<std::string> lines;
    std::ifstream file(path.c_str());
    std::string tempStr;
    while (std::getline(file, tempStr))
        lines.push_back(tempStr);

    //Extraction of lines without comments
    for (size_t i = 0; i < lines.size(); i++) {
        size_t idx = lines[i].find("//");
        if (idx != std::string::npos)
            lines[i] = lines[i].substr(0, idx);
    }

    objects.clear();
    bool parseSuccess;
    for (size_t i = 0; i < lines.size(); i++) {
        //std::cout << "Ltm.->Parsing line: " << lines[i] << std::endl;
        std::vector<std::string> parts;
        std::vector<std::string> loc;
        boost::split(parts, lines[i], boost::is_any_of(" ,\t"),
                boost::token_compress_on);
        if (parts.size() < 3)
            continue;
        //std::cout << "Ltm.->Parsing splitted line: " << lines[i] << std::endl;
        parseSuccess = true;
        loc.push_back(parts[1]);
        loc.push_back(parts[2]);
        loc.push_back(parts[3]);
        loc.push_back(parts[4]);
        loc.push_back(parts[5]);
        loc.push_back(parts[6]);
        loc.push_back(parts[7]);

        if (parseSuccess) {
            //std::cout << "LOCATIONS" << parts[0] << std::endl;
            objects[parts[0]] = loc;
        }
    }
    std::cout << "Ltm.->Total number of known locations: " << objects.size() << std::endl;
    for (std::map<std::string, std::vector<std::string> >::iterator it = objects.begin(); it != objects.end(); it++) {
        std::cout << "Ltm.->Location " << it->first << " " << it->second[0] << " " << it->second[1];
        if (it->second.size() > 2)
            std::cout << " " << it->second[2];
        std::cout << std::endl;
    }
    if (objects.size() < 1)
        std::cout << "Ltm.->WARNING: Cannot load known locations from file: "
            << path << ". There are no known locations." << std::endl;

}

void JustinaRepresentation::addLocations(std::map<std::string, std::vector<std::string> >& locations, std::string name, std::vector<std::string> values)
{
    std::map<std::string, std::vector<std::string> >::iterator it;

    it = locations.find(name);
    if (it != locations.end()) {
        it->second[0] = values.at(0);
        it->second[1] = values.at(1);
        it->second[2] = values.at(2);
    }
    else
        locations[name] = values;

}

void JustinaRepresentation::addObjects(std::map<std::string, std::vector<std::string> >& objects, std::string name, std::vector<std::string> values)
{
    std::map<std::string, std::vector<std::string> >::iterator it;

    it = objects.find(name);
    if (it != objects.end()) {
        it->second[0] = values.at(0);
        it->second[1] = values.at(1);
        it->second[2] = values.at(2);
        it->second[3] = values.at(3);
        it->second[4] = values.at(4);
        it->second[5] = values.at(5);
        it->second[6] = values.at(6);
    }
    else
        objects[name] = values;

}

bool JustinaRepresentation::speachInterpretation(){
    std::string testPrompt = "JustinaRepresentation.->";
    bool success = ros::service::waitForService("/planning_clips/spr_interpreter", 5000);
    if (success) {
        knowledge_msgs::planning_cmd srv;
        srv.request.name = "test_interprete";
        srv.request.params = "Ready to interpretation";
        if (cliSpechInterpretation->call(srv)) {
            std::cout << "Response of interpreter:" << std::endl;
            std::cout << "Success:" << (long int) srv.response.success
                << std::endl;
            std::cout << "Args:" << srv.response.args << std::endl;
            return (bool) srv.response.success;
        }
        std::cout << testPrompt << "Failed to call service of interpreter"
            << std::endl;
        return false;
    }
    std::cout << testPrompt << "Needed services are not available :'("
        << std::endl;
    return false;
}

bool JustinaRepresentation::stringInterpretation(std::string strToInterpretation, std::string &strInterpreted){
    std::string testPrompt = "JustinaRepresentation.->";
    bool success = ros::service::waitForService("/planning_clips/str_interpreter", 5000);
    if (success) {
        knowledge_msgs::planning_cmd srv;
        srv.request.name = "test_interprete";
        srv.request.params = strToInterpretation;
        if (cliStringInterpretation->call(srv)) {
            std::cout << "Response of interpreter:" << std::endl;
            std::cout << "Success:" << (long int) srv.response.success
                << std::endl;
            std::cout << "Args:" << srv.response.args << std::endl;
            strInterpreted = srv.response.args;
            return (bool) srv.response.success;
        }
        std::cout << testPrompt << "Failed to call service of interpreter"
            << std::endl;
        return false;
    }
    std::cout << testPrompt << "Needed services are not available :'("
        << std::endl;
    return false;
}

bool JustinaRepresentation::prepareInterpretedQuestionToQuery(std::string strInterpreted, std::string &query){
    std::size_t index = strInterpreted.find("(task");
    std::stringstream ss;
    ss << "(assert (";
    bool success;
    std::vector<std::string> tokens;
    if(index != std::string::npos)
        strInterpreted = strInterpreted.substr(index + 5 , strInterpreted.size());
    std::cout << "JustinaRepresentation.->New intepreted:" << strInterpreted << std::endl;
    boost::algorithm::split(tokens, strInterpreted, boost::algorithm::is_any_of("("));
    std::cout << "JustinaRepresentation.->Tokens size:" << tokens.size() << std::endl;
    for(int i = 0; i < tokens.size(); i++){
        std::cout << "JustinaRepresentation.->Token " << i << " :" << tokens[i] << std::endl;
        if(tokens[i].compare(" ") != 0){
            std::string tokenReplacement = tokens[i].substr(0 , tokens[i].size() - 2);
            std::vector<std::string> tokens_items;
            std::cout << "JustinaRepresentation.->Token Proc" << i << " :" << tokens[i] << std::endl;
            std::cout << "JustinaRepresentation.->Token cut" << i << " :" << tokenReplacement << std::endl;
            boost::algorithm::split(tokens_items, tokenReplacement, boost::algorithm::is_any_of(" "));
            if(tokens_items.size() >= 2){
                if(tokens_items[1].compare(")") != 0){
                    std::cout << "JustinaRepresentation.->Item token:" << tokens_items[0] << std::endl;
                    std::cout << "JustinaRepresentation.->Item token:" << tokens_items[1] << std::endl;
                    if(tokens_items[0].find("action_type") != std::string::npos && tokens_items[1].find("where_place") != std::string::npos){
                        success = true;
                        ss << "cmd_what_place ";
                    }
                    else if(tokens_items[0].compare("params") == 0){
                        for(int j = 1; j < tokens_items.size(); j++)
                            ss << tokens_items[j] << " ";
                    }
                }
            }
        }
    }
    if(success)
        ss << "1))";
    query = ss.str();
    std::cout << "JustinaRepresentation.->Query:" << query << std::endl;
    return success;
}

bool JustinaRepresentation::waitForQueryResult(int timeout, std::string &queryResultRef){
    queryResultReceive = false;
    ros::Rate rate(30);
    while(ros::ok() && !queryResultReceive){
        rate.sleep();
        ros::spinOnce();
    }
    queryResultRef = queryResult;
    return true;
}

void JustinaRepresentation::selectCategoryObjectByName(std::string idObject){
    std::stringstream ss;
    ss << "(assert (cmd_simple_category " << idObject << " 1))";
    JustinaRepresentation::sendAndRunCLIPS(ss.str());
}
