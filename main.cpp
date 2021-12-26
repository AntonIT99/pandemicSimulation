#include <iostream>
#include <fstream>
#include <ctime>
#include <locale>
#include <cmath>

using namespace std;

double calcFunction(int index, double s, double i, double v, double R, double T, double D, double V, double Veff)
{
    switch(index)
    {
        case 1: //s
            return -(R/T)*i*s - V;
            break;
        case 2: //i
            return ((R/T)*(s + (1-Veff)*v) - ((1+D)/T))*i;
            break;
        case 3: //r
            return (1/T)*i;
            break;
        case 4: //d
            return (D/T)*i;
            break;
        case 5: //cumulated i
            return (R/T)*(s + (1-Veff)*v)*i;
            break;
        case 6: //v
            return V - (R/T)*(1-Veff)*v*i;
            break;
        default:
            return 0;
            break;
    }
}

typedef struct DATE DATE;
struct DATE
{
    int year;
    int month;
    int day;

    bool operator==(const DATE& d) const
    {
        return (year == d.year && month == d.month && day == d.day);
    }

    bool operator>(const DATE& d) const
    {
        return (year > d.year || (year == d.year && month > d.month) || (year == d.year && month == d.month && day > d.day));
    }

    bool operator<(const DATE& d) const
    {
        return (year < d.year || (year == d.year && month < d.month) || (year == d.year && month == d.month && day < d.day));
    }

    bool operator>=(const DATE& d) const
    {
        return (year > d.year || (year == d.year && month > d.month) || (year == d.year && month == d.month && day >= d.day));
    }

    bool operator<=(const DATE& d) const
    {
        return (year < d.year || (year == d.year && month < d.month) || (year == d.year && month == d.month && day <= d.day));
    }
};

DATE nextDay(DATE currentDate)
{
    DATE nextDay;
    nextDay.day = currentDate.day;
    nextDay.month = currentDate.month;
    nextDay.year = currentDate.year;

    bool bissextile_year = false;

    if ((currentDate.year%4==0 && currentDate.year%100!=0) || currentDate.year%400==0)
    {
        bissextile_year = true;
    }

    if (currentDate.month == 12 && currentDate.day == 31)
    {
        nextDay.day = 1;
        nextDay.month = 1;
        nextDay.year++;
    }
    else if (currentDate.month == 2 && ((bissextile_year && currentDate.day == 29) || (!bissextile_year && currentDate.day == 28)))
    {
        nextDay.day = 1;
        nextDay.month++;
    }
    else if(currentDate.day == 30 && ((currentDate.month == 4) || (currentDate.month == 6) || (currentDate.month == 9) || (currentDate.month == 11)))
    {
        nextDay.day = 1;
        nextDay.month++;
    }
    else if(currentDate.day == 31 && ((currentDate.month == 1) || (currentDate.month == 3) || (currentDate.month == 5) || (currentDate.month == 7) || (currentDate.month == 8) || (currentDate.month == 10)))
    {
        nextDay.day = 1;
        nextDay.month++;
    }
    else
    {
        nextDay.day++;
    }

    return nextDay;
}

DATE date_add_days(int days, DATE date)
{
    if (days >= 1)
    {
        return date_add_days(days-1, nextDay(date));
    }
    else
    {
        return date;
    }
}

int number_of_digits(int nb)
{
    return (nb != 0 ? (int)(log10(abs(nb))) + 1 : 1);
}

string number_with_separator(int n, string separator)
{
    string s = "";
    if (n<0) s = "-";
    n = abs(n);
    int i_temp = (number_of_digits(n)-1)/3;

    for(int i = i_temp*3; i > 0; i-=3)
    {
        int threeDigits = n/(int)pow(10,i);
        for(int j = 3-number_of_digits(threeDigits); j > 0 && i < i_temp*3; j--)
        {
            s+=to_string(0);
        }
        s += to_string(threeDigits) + separator;
        n = n%(int)pow(10,i);
    }
    for(int i = 3-number_of_digits(n); i > 0 && i_temp >= 1; i--)
    {
        s+=to_string(0);
    }

    s+=to_string(n);

    return s;
}

bool XOR(bool condition1, bool condition2)
{
    return condition1 ? !condition2 : condition2;
}

int main()
{
    ofstream simFile, simFile2;
    double population = 83000000;
    double R0 = 2.5; //Basic reproduction number
    double Rtime = 10; //time of recovery
    double Drate = 0.002; //death rate
    double simTime = -1; //simulation time in days, set to -1 to have the simulation running until the pandemic is over
    double timeStep = 0.00001; //time step for calculations, the smaller is the step, the more precise the results will be, but the computing time will increase. timeStep = 0.00001 will ensure almost no errors at all.
    double nb0 = 1;//number of patients zero
    double Nv = 0; //vaccinations per day
    double Vr = Nv/population; //vaccination rate per day
    double Reff = R0; //Reproduction number
    double Rlockdown = 0.9; //Reproduction number in the lockdown
    double tlockdown = 1; //duration in days to lower/increase Reff to its new value
    double vaccinated_people = 0; //amount of people vaccinated prior to the simulation start
    double v_eff = 0.9; //efficiency of the vaccine from 0 to 1 (0-100%)

    //trigger lockdowns by cases
    int lockdown_duration = 90; //maximum duration in days. Set to 0 for no lockdowns
    int minium_lockdown_duration = 60; //minimum duration in days
    int casesTriggerLockdown = 200000;
    int casesRemoveLockdown = 10000; //set to 0 if lockdown should have a fixed duration (max duration)


    DATE startingDate; //starting date of the simulation
    startingDate.year = 2019;
    startingDate.month = 10;
    startingDate.day = 01;

    /*startingDate.year = 2020;
    startingDate.month = 1;
    startingDate.day = 24;*/

    DATE vaccineDate; //date from which vaccination becomes available
    vaccineDate.year = 2020;
    vaccineDate.month = 12;
    vaccineDate.day = 20;

    //do not change these default values below
    DATE lockdown_begin, lockdown_end, lockdown_canEnd;
    lockdown_end.year=2030;
    lockdown_end.month=1;
    lockdown_end.day=1;
    lockdown_canEnd.year=2030;
    lockdown_canEnd.month=1;
    lockdown_canEnd.day=1;

    DATE date = startingDate; //current date

    //initial parameters of the SIRD model - do not change them
    double s = (population-nb0-vaccinated_people)/(population);
    double i = nb0/(population);
    double r = 0;
    double d = 0;
    double v = vaccinated_people/population;
    double Vrate = 0; //vaccination rate
    double nI = 0; //Faction of the people who is or was ever infected (cumulated infectious cases)
    double sk, ik, vk; //parameters used for the Runge–Kutta second-order method
    double kl = -1; //lockdown time step counter

    int sp = (int) round(population * s);
    int ip = (int) round(population * i);
    int rp = (int) round(population * r);
    int dp = (int) round(population * d);
    int vp = (int) round(population * v);
    int cummulatedCases = 0;
    int newCases = 0;
    int Recoveries = 0;
    int Deaths = 0;
    int days = 0;
    int  nI_temp = 0, r_temp = 0; //variables used to calculate new cases and recoveries every day

    bool lockdown = false;

    if (date == vaccineDate) Vrate = Vr;

    simFile.open("pandemic.dat");
    simFile2.open("pandemic2.dat");

    simFile << date.day << "/" << date.month << "/" << date.year << " 00:00:00 " << sp << " " << ip << " " << rp << " " << dp << " " << vp << " " << cummulatedCases << endl;
    simFile2 << date.day << "/" << date.month << "/" << date.year << " 00:00:00 " << newCases << " " << Recoveries << " " << Deaths << " " << (int)(lockdown)*population/2 << " " << Reff << " " << (int)(lockdown) << endl;

    for (int k = 0; XOR(((k < (int)(simTime/timeStep)) && simTime>0), (simTime<0 && ip>=1)); k++)
    {
        if (lockdown) //lockdown effects
        {
            if (kl/(1/timeStep) < tlockdown) kl++;
            if (tlockdown != 0)  Reff = ((Rlockdown - R0)/tlockdown)*(kl/(1/timeStep)) + R0;
            else Reff = Rlockdown;
        }
        else if (!lockdown && kl >= 0) //removing lockdown effects
        {
            if (kl/(1/timeStep) < tlockdown) kl++;
            if (tlockdown != 0) Reff = ((R0 - Rlockdown)/tlockdown)*(kl/(1/timeStep)) + Rlockdown;
            else Reff = R0;
        }

        ///Runge–Kutta second-order method
        sk = s + timeStep*calcFunction(1, s, i, v, Reff, Rtime, Drate, Vrate, v_eff);
        ik = i + timeStep*calcFunction(2, s, i, v, Reff, Rtime, Drate, Vrate, v_eff);
        vk = v + timeStep*calcFunction(6, s, i, v, Reff, Rtime, Drate, Vrate, v_eff);

        s = s + timeStep*(calcFunction(1, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(1, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;

        if (s < 0) //vaccination evolution is linear and it need to be stopped if there are no one left to vaccinate
        {
            s = 0;
            Vrate = 0;
        }

        v = v + timeStep*(calcFunction(6, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(6, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;
        i = i + timeStep*(calcFunction(2, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(2, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;
        r = r + timeStep*(calcFunction(3, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(3, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;
        d = d + timeStep*(calcFunction(4, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(4, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;
        nI = nI + timeStep*(calcFunction(5, s, i, v, Reff, Rtime, Drate, Vrate, v_eff)+calcFunction(5, sk, ik, vk, Reff, Rtime, Drate, Vrate, v_eff))/2;

        if (k%((int)(1/timeStep)) == 0) //for each day
        {
            days++;
            date = nextDay(date);
            if (date == vaccineDate && s > 0) Vrate = Vr;

            Deaths = dp;

            sp = (int) round(population * s);
            ip = (int) round(population * i);
            rp = (int) round(population * r);
            dp = (int) round(population * d);
            vp = (int) round(population * v);
            cummulatedCases = (int) round(population * nI);

            newCases = cummulatedCases - nI_temp;
            Recoveries = rp - r_temp;
            Deaths = dp - Deaths;

            nI_temp = cummulatedCases;
            r_temp = rp;

            if (!lockdown && newCases > casesTriggerLockdown && lockdown_duration > 0) //triggering lockdown
            {
                kl = 0;
                lockdown = true;
                lockdown_begin = date;
                lockdown_end = date_add_days(lockdown_duration, lockdown_begin);
                lockdown_canEnd = date_add_days(minium_lockdown_duration, lockdown_begin);
            }
            if (lockdown && ((newCases < casesRemoveLockdown && date >= lockdown_canEnd) || date >= lockdown_end)) //removing lockdown
            {
                kl = 0;
                lockdown = false;
                lockdown_end = date;
            }

            simFile << date.day << "/" << date.month << "/" << date.year << " 00:00:00 " << sp << " " << ip << " " << rp << " " << dp << " " << vp << " " << cummulatedCases << endl;
            simFile2 << date.day << "/" << date.month << "/" << date.year << " 00:00:00 " << newCases << " " << Recoveries << " " << Deaths << " " << (int)(lockdown)*population/2 << " " << Reff << " " << (int)(lockdown) << endl;
        }
    }

    cout << "---END-OF-SIMULATION---" << endl;
    cout << endl << "Starting date: " << startingDate.day << "/" << startingDate.month << "/" << startingDate.year << endl;
    cout << "End Date: " << date.day << "/" << date.month << "/" << date.year << endl;
    cout << "Duration: " << number_with_separator(days, ",") << " days" << endl;
    cout << "Susceptible: " << number_with_separator(sp, ",") << endl;
    cout << "Infectious: " << number_with_separator(ip, ",") << endl;
    cout << "Recovered: " << number_with_separator(rp, ",") << endl;
    cout << "Deceased: " << number_with_separator(dp, ",") << endl;
    cout << "Vaccinated: " << number_with_separator(vp, ",") << endl;
    cout << "Required Herd Immunity level: " << (1-(1/R0))*100 << "%" << endl;
    cout << "Achieved Herd Immunity level without vaccination: " << r*100 << "%" << endl;
    cout << "Achieved Herd Immunity level with vaccination: " << (r+v_eff*v)*100 << "%" << endl;

    system("gnuplot -e \"load \'plot.gp\'\" --persist");

    simFile.close();
    return 0;
}
