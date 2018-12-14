//<LICENSE>

#include "imagingmoduleconfigurator.h"
#include <sstream>
#include <ConfigBase.h>
#include <ImagingException.h>
#include <ModuleException.h>
#include <base/KiplException.h>
#include <KiplFactory.h>
#include <KiplEngine.h>
#include "ImageIO.h"

ImagingModuleConfigurator::ImagingModuleConfigurator(KiplProcessConfig *config)
{
    std::cout << "ImagingModuleConfigurator::ImagingModuleConfigurator" << std::endl;
    ModuleConfigurator::m_Config=dynamic_cast<ConfigBase *>(config);
}

ImagingModuleConfigurator::~ImagingModuleConfigurator(){

}


int ImagingModuleConfigurator::GetImage(std::string sSelectedModule)
{
    std::cout << "ImagingModuleConfigurator::GetImage" << std::endl;
    KiplEngine *engine=nullptr;
    KiplFactory factory;

    KiplProcessConfig * config=dynamic_cast<KiplProcessConfig *>(m_Config);
    std::ostringstream msg;
    try {
        engine=factory.BuildEngine(*dynamic_cast<KiplProcessConfig *>(m_Config));
    }
    catch (ImagingException &e) {
        msg<<"Failed to build the configuration engine with a ImagingException: "<<e.what();
    }
    catch (ModuleException &e){
        msg<<"Failed to build the configuration engine with a ModuleException: "<<e.what();
    }
    catch (kipl::base::KiplException &e) {
        msg<<"Failed to build the configuration engine with a KiplException: "<<e.what();
    }

    catch (std::exception &e) {
        msg<<"Failed to build the configuration engine with a STL Exception: "<<e.what();
    }
    catch (...) {
        msg<<"Failed to build the configuration engine with an unhandled ";
    }
    if (!msg.str().empty()) {
        throw ModuleException(msg.str(),__FILE__,__LINE__);
    }

    logger(kipl::logging::Logger::LogMessage,"Engine successfully built");

    std::cout << "before load volume image" << std::endl;
    m_OriginalImage = LoadVolumeImage(*config);

    std::cout << "before run preproc" << std::endl;
    m_Image=engine->RunPreproc(&m_OriginalImage,sSelectedModule);

    std::cout << "before delete engin" << std::endl;

    if (engine!=nullptr)
        delete engine;
    return 0;
}

