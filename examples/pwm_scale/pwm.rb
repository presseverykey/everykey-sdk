
# timefreq = clockin / (prescalar * compare)


def timerfreq (clock, pre, comp) 
  clock / (pre * comp).to_f
end

def pre_compar (goalfreq, clock, bits) 
  max = (2 ** bits)-1
  goalfreq = goalfreq.to_f


  min_delta = 9999.0
  best = []


  min_prescalar = timerfreq(clock, goalfreq, max).ceil
  max_prescalar = timerfreq(clock, goalfreq, 1).floor
  max = max > max_prescalar ? max_prescalar : max

  min_prescalar.upto(max) {|pre| 
    comp = clock / (goalfreq * pre).floor
    actualfreq = timerfreq(clock, pre, comp)
    delta = (actualfreq - goalfreq).abs
    if delta < min_delta
      best = [pre,comp,actualfreq, delta]
      min_delta = delta
    end

    comp = comp+1
    actualfreq = timerfreq(clock, pre, comp)
    delta = (actualfreq - goalfreq).abs
    if delta < min_delta
      best = [pre,comp,actualfreq,delta]
      min_delta = delta
    end
  } 
  return best
end

LPC_CLOCK = 72000000


def usage
  STDERR.puts ("usage: <ruby> #{$0} ...")
  STDERR.puts ("  calculate prescalar and compare values for a")
  STDERR.puts ("  goal frequency")
  STDERR.puts ("-f [goal frequency]")
  STDERR.puts ("-b number of bits in prescale / compare values (default 16)")
  STDERR.puts ("-c clock speed (default 72000000)")
  exit(1)
end

if $0 == __FILE__
  if ARGV.length == 0 || ARGV.length == 1
    usage
  end
 ARGV.each_with_index{|a,i|
  puts "#{i} #{a}"
 }
  0.step(ARGV.length-1, 2) {|i|
    case ARGV[i]
    when "-f" 
      @frequency = ARGV[i+1].to_f
    when "-b" 
      @bits      = ARGV[i+1].to_i
    when "-c" 
      @clock     = ARGV[i+1].to_i
    else 
      usage
    end
  }
puts :here
  @bits  ||= 16
  @clock ||= LPC_CLOCK
  
  if !@frequency
    usage
  end

  puts "Calculation prescaler and compare for:"
  puts "  goal frequency: #{@frequency}"
  puts "  system clock  : #{@clock}"
  puts "  comp/prescale : #{@bites} bits"
  puts "--------------------------------------"
  p_c = pre_compar(@frequency, @clock, @bits)
  puts "suggested"
  puts "  prescalar     : #{p_c[0]}"
  puts "  compare       : #{p_c[1]}"
  puts "  actual freq   : #{p_c[2]}"
  puts "  delta         : #{p_c[3]} (#{100* (p_c[3]/@frequency.to_f)}%)"
  puts "(no gurantees, use at your own discretion!"
end
