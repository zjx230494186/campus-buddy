package com.campusbuddy.database;

import org.flywaydb.core.Flyway;
import org.flywaydb.core.api.MigrationInfo;
import org.junit.jupiter.api.Test;
import org.testcontainers.containers.PostgreSQLContainer;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;

import java.sql.DriverManager;

import static org.assertj.core.api.Assertions.assertThat;

@Testcontainers
class DatabaseMigrationTest {

    @Container
    static final PostgreSQLContainer<?> postgres = new PostgreSQLContainer<>("postgres:17.9");

    @Test
    void flywayMigrationCreatesTechnicalSpikeMarkerTable() throws Exception {
        Flyway flyway = Flyway.configure()
                .dataSource(postgres.getJdbcUrl(), postgres.getUsername(), postgres.getPassword())
                .locations("classpath:db/migration")
                .load();

        flyway.migrate();

        assertThat(flyway.info().applied())
                .extracting(MigrationInfo::getVersion)
                .extracting(Object::toString)
                .contains("1");

        try (var connection = DriverManager.getConnection(
                postgres.getJdbcUrl(),
                postgres.getUsername(),
                postgres.getPassword());
             var tableStatement = connection.prepareStatement("""
                     select exists (
                         select 1
                         from information_schema.tables
                         where table_schema = 'public'
                           and table_name = 'technical_spike_marker'
                     )
                     """);
             var markerStatement = connection.prepareStatement("""
                     select marker_key, marker_value
                     from technical_spike_marker
                     where marker_key = 'database_migration'
                     """)) {
            try (var tableResult = tableStatement.executeQuery()) {
                assertThat(tableResult.next()).isTrue();
                assertThat(tableResult.getBoolean(1)).isTrue();
            }

            try (var markerResult = markerStatement.executeQuery()) {
                assertThat(markerResult.next()).isTrue();
                assertThat(markerResult.getString("marker_value")).isEqualTo("flyway_testcontainers_postgresql");
            }
        }
    }
}
