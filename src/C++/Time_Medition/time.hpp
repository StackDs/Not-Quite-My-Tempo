#pragma once

#include <stdexcept>
#include <chrono>

enum class TimeUnit{
    nanoseconds,
    microseconds,
    milliseconds,
    seconds,
    minutes,
    hours
};


class time {
private:
    //Usamos un un reloj monotonico
    usign clock = std::chrono::steady_clock

    // Variables de calculo 
    clock::time_point start_time_{};
    clock::duration measured_duration_{};

    // Booleanos de comprobacion
    bool running_{false};
    bool has_measurement_{false};

public:

// Funciones para iniciar y terminar (Actualizan las variables)
    void start(){
        if(running_){
            throw std::logic_error("El cronometro ya esta corriendo");
        }

        start_time_ = clock::now();
        running_ = true;
        has_measurement_ = true;

    }

    void stop(){
        if(!running_){
            throw std::logic_error("No se puede detener un cronometro que no ha empezado")
        }

        measured_duration_ = clock::now() - start_time_;
        running_ = false;
        has_measurement_ = true;
    }


    double elapsed(TimeUnit unit) const{
        if(running_){
            throw std::logic_error("Deten el cronometro antes de consultar el resultado")
        }
        if(!has_measurement_){
            throw std::logic_error("Todavia no hay una medicion disponible");
        }
        
        const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(measured_duration_).count();

        switch(unit){
             case TimeUnit::nanoseconds:
            return static_cast<double>(nanoseconds);
        case TimeUnit::microseconds:
            return static_cast<double>(nanoseconds) / 1000.0;
        case TimeUnit::milliseconds:
            return static_cast<double>(nanoseconds) / 1000000.0;
        case TimeUnit::seconds:
            return static_cast<double>(nanoseconds) / 1000000000.0;
        case TimeUnit::minutes:
            return static_cast<double>(nanoseconds) / 60000000000.0;
        case TimeUnit::hours:
            return static_cast<double>(nanoseconds) / 3600000000000.0;
     }

    throw std::invalid_argument("Unidad de tiempo no valida");

    }
};


