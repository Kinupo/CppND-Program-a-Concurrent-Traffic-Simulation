#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> message_quque_guard(_message_mutex);

    while(_queue.size() == 0) {
        _inturupt.wait(message_quque_guard);
    }

    auto message = _queue.front();
    _queue.pop_front();
        
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    std::lock_guard<std::mutex> message_queue_guard(_message_mutex);
    //clear need due to consumers not consuming all messages before
    //they become stale
    _queue.clear();
    _queue.emplace_back(msg);
    _inturupt.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::kRed;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(1){
        if(TrafficLightPhase::kGreen == _light_status_quque.receive())
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    _cycling_lights = std::thread(&TrafficLight::cycleThroughPhases, this);
}

TrafficLightPhase SwitchLightPhase(TrafficLightPhase current_light_status){
    if(current_light_status == kRed)
        return kGreen;
    else
        return kRed;

}

std::uniform_int_distribution<int> InitilizeRandomDevice(int lower_bound, int upper_bound){
    return std::uniform_int_distribution<int>(lower_bound, upper_bound);
}

std::chrono::milliseconds CalculateTimeDiffernce(std::chrono::_V2::system_clock::time_point& refrence_time){
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - refrence_time);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    std::default_random_engine generator;
    auto random_distributer = InitilizeRandomDevice(4000, 6000);

    auto wait_time = std::chrono::milliseconds(random_distributer(generator));
    auto time__light_switched = std::chrono::system_clock::now();

    while(true){
        
        std::this_thread::sleep_for(_cycle_wait_time);

        if(wait_time < CalculateTimeDiffernce(time__light_switched)){
            
            _currentPhase = SwitchLightPhase(_currentPhase);
            _light_status_quque.send(std::move(_currentPhase));

            wait_time = std::chrono::milliseconds(random_distributer(generator));
            time__light_switched = std::chrono::system_clock::now();
        }
    }
}