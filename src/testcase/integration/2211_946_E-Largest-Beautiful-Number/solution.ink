// Translated from solution.cpp.

class Timer
{
  var scope_name: dynamic;
  var start_time: dynamic;
  func Timer(name: dynamic)
  {
      this->scope_name = cpp_construct(name);
      start_time = chrono.high_resolution_clock.now();
    }
  func ~Timer()
  {
      var stop_time = chrono.high_resolution_clock.now();
      var length = chrono.duration_cast((stop_time - start_time)).count();
      var mlength = (double(length) * 0.001);
    }
}

var MOD = 1000000007;

var INF = 0x3f3f3f3f3f3f3f3f;

var iNF = 0x3f3f3f3f;

var t: dynamic;

var s: dynamic;

var cnt = cpp_array(11);

func pow10()
{
  var al = true;
  {
    var i = 1;
    while ((i < ((int_cpp(s.size()) - 2) + 1)))
    {
      al &= (s[i] == cpp_char("0"));
      i += 1;
    }
  }
  return ((al && (s.front() == cpp_char("1"))) && (((s.back() == cpp_char("1")) || (s.back() == cpp_char("0")))));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(s);
    if (pow10())
    {
      {
        var i = 0;
        while ((i < (int_cpp(s.size()) - 2)))
        {
          write(9);
          i += 1;
        }
      }
      write(cpp_char("\n"));
    } else
    {
      memset(cnt, (0), cpp_sizeof((cnt)));
      for (var c in s)
      {
        cnt[(c - cpp_char("0"))] += 1;
      }
      var ans = s;
      {
        var i = (int_cpp(s.size()) - 1);
        while ((i >= 0))
        {
          cnt[(s[i] - cpp_char("0"))] -= 1;
          var flag = false;
          if ((s[i] != cpp_char("0")))
          {
            {
              var ts = ((s[i] - cpp_char("0")) - 1);
              while ((ts >= 0))
              {
                ans[i] = (ts + cpp_char("0"));
                cnt[ts] += 1;
                var od: dynamic;
                {
                  var d = 0;
                  while ((d < 10))
                  {
                    if ((cnt[d] & 1))
                    {
                      od.insert(d);
                    }
                    d += 1;
                  }
                }
                cnt[ts] -= 1;
                var fre = ((int_cpp(s.size()) - i) - 1);
                if ((int_cpp(od.size()) > fre))
                {
                  ts -= 1;
                  continue;
                }
                if ((((fre - int_cpp(od.size()))) & 1))
                {
                  ts -= 1;
                  continue;
                }
                var odd: dynamic;
                for (var c in od)
                {
                  odd.emplace_back(c);
                }
                var x = (fre - int_cpp(od.size()));
                {
                  var j = 1;
                  while ((j < (x + 1)))
                  {
                    ans[(i + j)] = cpp_char("9");
                    j += 1;
                  }
                }
                {
                  var j = 0;
                  while ((j < int_cpp(od.size())))
                  {
                    ans[(((j + x) + i) + 1)] = (odd.back() + cpp_char("0"));
                    odd.pop_back();
                    j += 1;
                  }
                }
                write(ans, cpp_char("\n"));
                flag = true;
                break;
                ts -= 1;
              }
            }
          }
          if (flag)
          {
            break;
          }
          i -= 1;
        }
      }
    }
  }
  return 0;
}
