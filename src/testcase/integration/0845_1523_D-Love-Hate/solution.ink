// Translated from solution.cpp.

var INF = 1e9;

var MOD = (1e9 + 7);

var arr = cpp_array(65, 200100);

var cnt = cpp_array(33010);

var dp = cpp_array(33010);

func popcnt(msk: dynamic)
{
  var ret = 0;
  while ((msk > 0))
  {
    ret += 1;
    msk -= ((msk & (-msk)));
  }
  return ret;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var p: dynamic;
  var maxv = -1;
  var ans: dynamic;
  scanf("%d %d %d", (&n), (&m), (&p));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
    var t = 0;
    while ((t < 50))
    {
      var idx = ((((rand() * 30000) + rand())) % n);
      var curr: dynamic;
      var siz = 0;
      {
        var j = 0;
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
        var i = 0;
        while ((i < n))
        {
          var msk = 0;
          {
            var j = 0;
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
        var msk = 0;
        while ((msk < ((1 << siz))))
        {
          {
            var smsk = msk;
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
        var smsk = 0;
        while ((smsk < ((1 << siz))))
        {
          if ((dp[smsk] >= (((n + 1)) / 2)))
          {
            if ((maxv < popcnt(smsk)))
            {
              var now = string_cpp(m, cpp_char("0"));
              {
                var j = 0;
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
