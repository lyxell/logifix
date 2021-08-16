import java.util.Date;
import java.time.ZoneId;
import java.time.ZonedDateTime;

public class TestDate {
    private static ZonedDateTime convert(Date date, ZoneId zoneId) {
        if (date instanceof java.sql.Date) {
            java.sql.Date sqlDate = (java.sql.Date) date;
            return sqlDate.toLocalDate().atStartOfDay(zoneId);
        }
        return date.toInstant().atZone(zoneId);
    }
}
