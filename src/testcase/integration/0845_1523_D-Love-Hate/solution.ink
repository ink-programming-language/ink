// Translated from solution.cpp.

var INF: dynamic = 1e9;

var MOD: dynamic = (1e9 + 7);

var arr: dynamic = cpp_array(65, 200100);

var cnt: dynamic = cpp_array(33010);

var dp: dynamic = cpp_array(33010);

func popcnt(msk: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while ((msk > 0))
  {
    ret += 1;
    msk -= ((msk & (-msk)));
  }
  return ret;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var maxv: dynamic = -1;
  var ans: dynamic = cpp_uninitialized();
  scanf("%d %d %d", (&n), (&m), (&p));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          scanf("%1d", (&arr[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  srand(time(null));
  {
    var t: dynamic = 0;
    while ((t < 50))
    {
      var idx: dynamic = ((((rand() * 30000) + rand())) % n);
      var curr: dynamic = cpp_uninitialized();
      var siz: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if (arr[idx][j])
          {
            curr.push_back(j);
            siz += 1;
          }
          j += 1;
        }
      }
      memset(cnt, 0, cpp_sizeof((cnt)));
      memset(dp, 0, cpp_sizeof((dp)));
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var msk: dynamic = 0;
          {
            var j: dynamic = 0;
            while ((j < siz))
            {
              if (arr[i][curr[j]])
              {
                msk += ((1 << j));
              }
              j += 1;
            }
          }
          cnt[msk] += 1;
          i += 1;
        }
      }
      {
        var msk: dynamic = 0;
        while ((msk < ((1 << siz))))
        {
          {
            var smsk: dynamic = msk;
            while (true)
            {
              dp[smsk] += cnt[msk];
              if ((smsk == 0))
              {
                break;
              }
              smsk = ((((smsk - 1)) & msk));
            }
          }
          msk += 1;
        }
      }
      {
        var smsk: dynamic = 0;
        while ((smsk < ((1 << siz))))
        {
          if ((dp[smsk] >= (((n + 1)) / 2)))
          {
            if ((maxv < popcnt(smsk)))
            {
              var now: dynamic = string_cpp(m, cpp_char("0"));
              {
                var j: dynamic = 0;
                while ((j < siz))
                {
                  if ((smsk & ((1 << j))))
                  {
                    now[curr[j]] = cpp_char("1");
                  }
                  j += 1;
                }
              }
              maxv = popcnt(smsk);
              ans = now;
            }
          }
          smsk += 1;
        }
      }
      t += 1;
    }
  }
  printf("%s\n", ans.c_str());
  return 0;
}
