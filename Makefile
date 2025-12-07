CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = voting_system
SOURCES = main.cpp VotingSystem.cpp IntelligenceEngine.cpp AdvancedAnalytics.cpp AdvancedAnalytics_Part2.cpp AdvancedAnalytics_Part3.cpp ConsistencyScorer.cpp AntiAbuseEngine.cpp EnsembleModels.cpp StreamProcessor.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# CrowdDecision components
CROWDDECISION_OBJECTS = ConsistencyScorer.o AntiAbuseEngine.o EnsembleModels.o StreamProcessor.o

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET) demo_test demo_test.o intelligence_demo intelligence_demo.o setup_recommendations setup_recommendations.o advanced_demo advanced_demo.o custom_analysis custom_analysis.o crowddecision_demo crowddecision_demo.o $(CROWDDECISION_OBJECTS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Build and run tests
test: demo_test
	./demo_test

# Build test executable
demo_test: demo_test.o VotingSystem.o IntelligenceEngine.o
	$(CXX) $(CXXFLAGS) -o demo_test demo_test.o VotingSystem.o IntelligenceEngine.o

# Build and run intelligence demo
intelligence: intelligence_demo
	./intelligence_demo

# Build intelligence demo executable
intelligence_demo: intelligence_demo.o VotingSystem.o IntelligenceEngine.o
	$(CXX) $(CXXFLAGS) -o intelligence_demo intelligence_demo.o VotingSystem.o IntelligenceEngine.o

# Build and run setup recommendations demo
setup: setup_recommendations
	./setup_recommendations

# Build setup recommendations executable
setup_recommendations: setup_recommendations.o VotingSystem.o IntelligenceEngine.o
	$(CXX) $(CXXFLAGS) -o setup_recommendations setup_recommendations.o VotingSystem.o IntelligenceEngine.o

# Build and run advanced analytics demo
advanced: advanced_demo
	./advanced_demo

# Build advanced analytics demo executable
advanced_demo: advanced_demo.o VotingSystem.o IntelligenceEngine.o AdvancedAnalytics.o AdvancedAnalytics_Part2.o AdvancedAnalytics_Part3.o
	$(CXX) $(CXXFLAGS) -o advanced_demo advanced_demo.o VotingSystem.o IntelligenceEngine.o AdvancedAnalytics.o AdvancedAnalytics_Part2.o AdvancedAnalytics_Part3.o

# Build and run custom analysis (your own proposals)
custom: custom_analysis
	./custom_analysis

# Build custom analysis executable
custom_analysis: custom_analysis.o VotingSystem.o IntelligenceEngine.o AdvancedAnalytics.o AdvancedAnalytics_Part2.o AdvancedAnalytics_Part3.o
	$(CXX) $(CXXFLAGS) -o custom_analysis custom_analysis.o VotingSystem.o IntelligenceEngine.o AdvancedAnalytics.o AdvancedAnalytics_Part2.o AdvancedAnalytics_Part3.o

# Build and run CrowdDecision comprehensive demo
crowddecision: crowddecision_demo
	./crowddecision_demo

# Build CrowdDecision demo executable
crowddecision_demo: crowddecision_demo.o VotingSystem.o IntelligenceEngine.o $(CROWDDECISION_OBJECTS)
	$(CXX) $(CXXFLAGS) -o crowddecision_demo crowddecision_demo.o VotingSystem.o IntelligenceEngine.o $(CROWDDECISION_OBJECTS)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Install dependencies (if needed)
install:
	@echo "No external dependencies required"

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build the voting system (default)"
	@echo "  clean        - Remove build files"
	@echo "  run          - Build and run the program"
	@echo "  test         - Build and run demo tests"
	@echo "  intelligence - Build and run AI features demo"
	@echo "  setup        - Setup AI recommendations with sample data"
	@echo "  advanced     - Build and run advanced analytics demo"
	@echo "  custom       - Analyze YOUR OWN proposals interactively"
	@echo "  crowddecision- Build and run CrowdDecision comprehensive demo (NEW!)"
	@echo "  debug        - Build with debug symbols"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "CrowdDecision Demo showcases:"
	@echo "  • Weight-based consistency & stability scoring"
	@echo "  • Anti-abuse engine (bot & collusion detection)"
	@echo "  • Ensemble ML models (Naive Bayes + Random Forest)"
	@echo "  • Stream processing architecture"
	@echo "  • Full system integration"

.PHONY: all clean run test intelligence setup advanced custom crowddecision debug install help
