// Translated from solution.cpp.

var md = 1000000007;

var maxn = 1100;

var inf = 2020202020202020202;

class box
{
  var in_cpp: dynamic;
  var out: dynamic;
  var w: dynamic;
  var s: dynamic;
  var v: dynamic;
}

var dp = cpp_array(1100, 1100);

var n: dynamic;

var s: dynamic;

var subdp = cpp_array(1100);

var nice: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  return ((a.in_cpp < b.in_cpp) || ((a.in_cpp == b.in_cpp) && (a.out > b.out)));
}

func main()
{
  read(n, s);
  {
    var i = 0;
    while ((i < n))
    {
      var j: dynamic;
      read(j.in_cpp, j.out, j.w, j.s, j.v);
      j.in_cpp += 1;
      j.out += 1;
      nice.push_back(j);
      i += 1;
    }
  }
  var jj = [0, 1002, 0, s, md];
  nice.push_back(jj);
  sort(nice.begin(), nice.end(), (&cmp));
  {
    var i = n;
    while ((i >= 0))
    {
      {
        var pr = 0;
        while ((pr < (s + 1)))
        {
          var prn = min(nice[i].s, (pr - nice[i].w));
          if ((prn >= 0))
          {
            dp[i][pr] += nice[i].v;
          }
          if ((prn >= 0))
          {
            {
              var it = nice[i].in_cpp;
              while ((it <= nice[i].out))
              {
                subdp[it] = 0;
                it += 1;
              }
            }
            var curr = 0;
            {
              var u = n;
              while ((u > i))
              {
                if ((((nice[i].in_cpp <= nice[u].in_cpp)) && ((nice[i].out >= nice[u].out))))
                {
                  var x = (subdp[nice[u].out] + dp[u][prn]);
                  if ((x > curr))
                  {
                    curr = x;
                  }
                  subdp[nice[u].in_cpp] = curr;
                }
                var y = nice[u].in_cpp;
                while ((nice[(u - 1)].in_cpp <= y))
                {
                  subdp[y] = curr;
                  y -= 1;
                }
                u -= 1;
              }
            }
            dp[i][pr] += subdp[nice[i].in_cpp];
          }
          pr += 1;
        }
      }
      i -= 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < (s + 1)))
    {
      ans = max(ans, dp[0][i]);
      i += 1;
    }
  }
  write((ans - md));
  return 0;
}
