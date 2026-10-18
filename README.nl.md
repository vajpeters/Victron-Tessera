# Victron Tessera

Een native Tessera-dashboard voor je eigen Victron-sensoren in Home Assistant.
De eerste versie richt zich op een 10,1-inchscherm en een eenfase-installatie.

## Instellen

1. Zorg dat je Victron-sensoren in Home Assistant de juiste waarden tonen.
2. Voeg in Tessera de link `https://github.com/vajpeters/Victron-Tessera` toe bij Plugins.
3. Kies je scherm en koppel zelf de sensoren voor accupercentage, accuvermogen, totale zonneproductie,
   AC-verbruik en netvermogen. Spanning, stroom en kritieke verbruikers zijn optioneel; netvermogen ook zonder netaansluiting.
4. Kies **Opslaan en bouwen**. Een andere sensorkoppeling vereist een nieuwe build.
5. Plaats de dashboardtegel in je indeling, bij voorkeur groot op het 10,1-inchscherm. Tik erop voor details.
6. Pas indien nodig de tekenrichting van accu- en netvermogen aan bij de plugininstellingen.

De plugin gebruikt de eenheden van de gekozen sensoren. Vermogen mag W, kW of MW zijn; kWh en VA worden niet als W gelezen.
Zonder bron-tijdstempel staat er bewust dat de leeftijd onbekend is. De optionele tijdstempel moet epochseconden bevatten
en de gekozen meetset dateren. Vul hier geen gewone kloksensor in.

Je hebt in deze plugin geen VRM-token nodig. Die koppeling blijft bij je bestaande Home Assistant-integratie.
Deze versie telt geen fasen of zonnebronnen op. Koppel daarvoor zelf een juiste totaalsensor.
Zie de Engelse README voor de volledige eenheden, beperkingen en ontwikkelinstructies.

## Victron-indeling

Versie 0.2.0 toont op ruime tegels en detailkaarten de Victron-indeling: Net, AC-verbruik en Kritieke verbruikers bovenaan, een omvormersymbool in het midden, accu en zon/PV onderaan. Kritieke verbruikers heeft een eigen optionele sensorkeuze; deze wordt niet opgeteld bij AC-verbruik. Kleine tegels behouden de compacte weergave.

Pijlen verschijnen alleen bij geldige, verse metingen boven 1 W. Het omvormersymbool meldt geen gemeten bedrijfsstatus. Historische grafieken, MPPT-details en overige gegevens uit het voorbeeld worden niet verzonnen. Sla op en bouw opnieuw om de extra sensorkeuze toe te voegen.

De optionele bron-tijdstempel komt uit het attribuut `victron_timestamp` van de gekozen accupercentagesensor (gehele epochseconden). Dit moet de echte meettijd van de volledige gegevensset zijn; zonder dit attribuut meldt het scherm **Leeftijd onbekend**. Zo blijven alle acht sensorkeuzes binnen de limiet van Tessera. Een Home Assistant-templatesensor kan dit attribuut doorgeven.
