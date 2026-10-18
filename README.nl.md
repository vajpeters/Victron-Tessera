# Victron Tessera

Een native Tessera-dashboard voor je eigen Victron-sensoren in Home Assistant. De eerste uitwerking richt zich op een 10,1-inchscherm en een eenfase-installatie, met een compacte weergave voor kleinere tegels.

![Firmwarepreview op 10,1 inch met testgegevens](docs/images/dashboard-light.png)

## Instellen

1. Zorg dat je Victron-sensoren in Home Assistant de juiste waarden en een `unit_of_measurement` hebben.
2. Voeg in Tessera bij **Plugins > Toevoegen met een link** deze URL toe: `https://github.com/vajpeters/Victron-Tessera`.
3. Kies je scherm en selecteer **Opslaan en bouwen**. Tijdens de firmwarebouw hoef je geen sensoren te kiezen.
4. Plaats **Victron-dashboard** in de indeling.
5. Kies in de instellingen van die tegel afzonderlijk de entiteiten voor **Net**, **AC-verbruik**, **Kritieke verbruikers**, **Accu** en **Zon / PV**.
6. Kies daar ook een van de gewone Tessera-achtergrondkleuren.

De sensorkeuzes en achtergrondkleur zijn onderdeel van de tegelindeling. Je kunt ze later wijzigen zonder de firmware opnieuw te bouwen. Iedere tegel kan eigen bronnen gebruiken.

De Accu-keuze accepteert een percentage-sensor (`%`) of een vermogenssensor (`W`, `kW` of `MW`). Met een percentage toont de kaart de laadstatus; met een vermogenssensor toont hij laden of ontladen. Kritieke verbruikers en Net mogen leeg blijven. Voor een driefasesysteem kies je bestaande totaalsensoren; deze versie telt fasen nog niet zelf op.

## Indeling en gegevens

Ruime tegels en de detailkaart tonen Net, AC-verbruik en Kritieke verbruikers bovenaan, een omvormersymbool in het midden, en Accu en Zon/PV onderaan. De plug-in ondersteunt Tessera's 5x5-paginaraster. Gebruik op een liggend 10,1-inchscherm bij voorkeur 5x4; kleinere tegels schakelen automatisch over op een compacte lijst.

Vermogen mag W, kW of MW zijn. kWh, VA, onbekende, onbeschikbare en niet-numerieke waarden worden niet als vermogen of nul behandeld. Zon, AC-verbruik en Kritieke verbruikers moeten niet-negatief zijn. Net is positief bij afname en negatief bij teruglevering. Accuvermogen is positief bij laden en negatief bij ontladen.

De plug-in telt geen fasen of zonnebronnen op en bestuurt geen Victron-apparatuur. Het omvormersymbool is alleen een onderdeel van het schema. Historische grafieken, MPPT-details en ontbrekende waarden worden niet verzonnen.

## Compatibiliteit

Versie 0.4.0 vereist Tessera plugin API 0.15 met ondersteuning voor entiteitskeuzes als tegelopties. De tegel reserveert 768 bytes in Tessera's indelingsbudget en gebruikt op ondersteunde schermen PSRAM voor zijn runtimegegevens.

De plug-in opent geen netwerkservice en bewaart geen VRM-token. Home Assistant verzorgt de Victron-koppeling; alleen de gekozen entiteits-ID's worden in de Tessera-indeling opgeslagen.

Zie [Development](docs/DEVELOPMENT.md), [Hardware acceptance](docs/HARDWARE_ACCEPTANCE.md) en [Architecture](docs/ARCHITECTURE.md).

## Licentie

MIT. Gebaseerd op de Tessera-pluginstructuur, copyright 2026 Max Gramser. De Tessera-firmware heeft een eigen AGPL-licentie.
