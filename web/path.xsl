<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
	<html>
	<body>
    <xsl:for-each select="event">
		<h2>Date: <xsl:value-of select="@datetime"/></h2>
		<xsl:for-each select="state">
			<h2>Id: <xsl:value-of select="@equipment-id"/></h2>
			<h2>Status: <xsl:value-of select="@status"/></h2>
			<h3>Indicators</h3>
			<table border="1">
				<tr bgcolor="#9acd32">
					<th style="text-align:left">Name</th>
					<th style="text-align:left">Value</th>
				</tr>
				<xsl:for-each select="indicator">
					<tr>
						<td><xsl:value-of select="@name"/></td>
						<td><xsl:value-of select="@value"/></td>
					</tr>
				</xsl:for-each>
			</table>
			<h3>Equipments</h3>
			<table border="1">
				<tr bgcolor="#9acd32">
					<th style="text-align:left">Equipments</th>
					<th style="text-align:left">Status</th>
					<th style="text-align:left">Indicator</th>
				</tr>
				<xsl:for-each select="equipment">
				<tr>
					<td><xsl:value-of select="@equipment-id"/></td>
					<td><xsl:value-of select="@status"/></td>
					<td>
						<xsl:if test="indicator">
							<table border="1">
								<tr bgcolor="#eaca10">
									<th style="text-align:left">Name</th>
									<th style="text-align:left">Value</th>
								</tr>
								<xsl:for-each select="indicator">   
								<tr>
									<td><xsl:value-of select="@name"/></td>
									<td><xsl:value-of select="@value"/></td>
								</tr>
								</xsl:for-each>
							</table>
						</xsl:if>
					</td>
				</tr>
				</xsl:for-each>
			</table>
		</xsl:for-each>
	</xsl:for-each>	
	</body>
	</html>
</xsl:template>
</xsl:stylesheet>
