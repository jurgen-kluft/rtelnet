package rtelnet

import (
	ccore "github.com/jurgen-kluft/ccore/package"
	denv "github.com/jurgen-kluft/go-ide/denv"
	rcore "github.com/jurgen-kluft/rcore/package"
	rwifi "github.com/jurgen-kluft/rwifi/package"
)

// rtelnet is a debug package for Arduino Esp32/Esp8266 projects.
const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "rtelnet"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	ccorepkg := ccore.GetPackage()
	rcorepkg := rcore.GetPackage()
	rwifipkg := rwifi.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(ccorepkg)
	mainpkg.AddPackage(rcorepkg)
	mainpkg.AddPackage(rwifipkg)

	// main library
	mainlib := denv.SetupDefaultCppLibProjectForArduino(mainpkg, name)
	mainlib.AddDependencies(ccorepkg.GetMainLib())
	mainlib.AddDependencies(rcorepkg.GetMainLib())
	mainlib.AddDependencies(rwifipkg.GetMainLib())

	mainpkg.AddMainLib(mainlib)
	return mainpkg
}
