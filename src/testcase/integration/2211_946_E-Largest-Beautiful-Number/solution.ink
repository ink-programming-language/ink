// Translated from solution.cpp.

class Timer
{
  var scope_name: dynamic = cpp_uninitialized();
  var start_time: dynamic = cpp_uninitialized();
  func Timer(name: dynamic) -> dynamic
  {
      self->scope_name = cpp_construct(name);
      start_time = chrono.high_resolution_clock.now();
    }
  func cpp_destruct_Timer() -> dynamic
  {
      var stop_time: dynamic = chrono.high_resolution_clock.now();
      var length: dynamic = chrono.duration_cast((stop_time - start_time)).count();
      var mlength: dynamic = (cpp_double(length) * 0.001);
    }
}

var MOD: dynamic = 1000000007;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var iNF: dynamic = 0x3f3f3f3f;

var t: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(11);

func pow10() -> dynamic
{
  var al: dynamic = true;
  {
    var i: dynamic = 1;
    while ((i < ((int_cpp(s.size()) - 2) + 1)))
    {
      al &= (s[i] == cpp_char("0"));
      i += 1;
    }
  }
  return ((al && (s.front() == cpp_char("1"))) && (((s.back() == cpp_char("1")) || (s.back() == cpp_char("0")))));
}

func main() -> dynamic
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
        var i: dynamic = 0;
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
      for (var c: dynamic in s)
      {
        cnt[(c - cpp_char("0"))] += 1;
      }
      var ans: dynamic = s;
      {
        var i: dynamic = (int_cpp(s.size()) - 1);
        while ((i >= 0))
        {
          cnt[(s[i] - cpp_char("0"))] -= 1;
          var flag: dynamic = false;
          if ((s[i] != cpp_char("0")))
          {
            {
              var ts: dynamic = ((s[i] - cpp_char("0")) - 1);
              while ((ts >= 0))
              {
                ans[i] = (ts + cpp_char("0"));
                cnt[ts] += 1;
                var od: dynamic = cpp_uninitialized();
                {
                  var d: dynamic = 0;
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
                var fre: dynamic = ((int_cpp(s.size()) - i) - 1);
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
                var odd: dynamic = cpp_uninitialized();
                for (var c: dynamic in od)
                {
                  odd.emplace_back(c);
                }
                var x: dynamic = (fre - int_cpp(od.size()));
                {
                  var j: dynamic = 1;
                  while ((j < (x + 1)))
                  {
                    ans[(i + j)] = cpp_char("9");
                    j += 1;
                  }
                }
                {
                  var j: dynamic = 0;
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
