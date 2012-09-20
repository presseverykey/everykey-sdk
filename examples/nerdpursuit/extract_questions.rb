
require 'json'

def list dir
  Dir.glob("#{dir}/*.json").each {|fn|
    jsons = File.read(fn)
    json = JSON.parse(jsons)
    trivia = json["question"]
    if %{ruby rails css}.include? trivia["sub_category"] 
      next
    end
    if trivia["category"] == "culture"
      next
    end
    #puts "#{trivia["text"]} ||  #{trivia["category"]} || #{trivia["sub_category"]}" unless trivia["category"] == "culture"
    puts "\t{"
    puts "\t\t \"#{trivia["text"]}\""
    puts "\t\t,\"#{trivia["a1"]}\""
    puts "\t\t,\"#{trivia["a2"]}\""
    puts "\t\t,\"#{trivia["a3"]}\""
    puts "\t\t,\"#{trivia["a4"]}\""
    puts "\t\t,#{trivia["right_answer"][1]}"
    puts "\t},"
  }
end

if $0 == __FILE__
  dir = ARGV[0]
  list(dir)
end
