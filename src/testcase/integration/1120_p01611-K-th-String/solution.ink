// Translated from solution.cpp.

var MAX_N = int_cpp((1e5 + 10));

var MAX_K = 30;

var INF = int64(2.05e18);

var N: dynamic;

var K: dynamic;

var L: dynamic;

var sa = cpp_array((MAX_N + 1));

var comb = cpp_array((MAX_K + 1), ((MAX_N + MAX_K) + 1));

func multi(x: dynamic, y: dynamic)
{
  return if ((((double(x) * y) < (INF * 1.3)))) (x * y) else INF;
}

func init()
{
  scanf("%d%d%lld", (&N), (&K), (&L));
  sa[0] = N;
  {
    var i = 1;
    while ((i <= N))
    {
      scanf("%d", (sa + i));
      sa[i] -= 1;
      i += 1;
    }
  }
}

func prepareCombination()
{
  {
    var n = 0;
    while ((n <= (N + K)))
    {
      comb[n][0] = 1;
      if ((n <= K))
      {
        comb[n][n] = 1;
      }
      {
        var k = 1;
        while ((k <= min(n, K)))
        {
          if (((n - 1) >= k))
          {
            comb[n][k] = min(INF, (comb[(n - 1)][k] + comb[(n - 1)][(k - 1)]));
          }
          k += 1;
        }
      }
      n += 1;
    }
  }
}

class State
{
  var from_cpp: dynamic;
  var to: dynamic;
  var ways: dynamic;
  func State()
  {
    }
  func State(from_cpp: dynamic, to: dynamic, ways: dynamic)
  {
      this->from_cpp = cpp_construct(from_cpp);
      this->to = cpp_construct(to);
      this->ways = cpp_construct(ways);
    }
}

func solve()
{
  prepareCombination();
  var inv = cpp_array((MAX_N + 1));
  {
    var i = 0;
    while ((i <= N))
    {
      inv[sa[i]] = i;
      i += 1;
    }
  }
  var mustInc = cpp_array((MAX_N + 1));
  var incCount = 0;
  {
    var i = 1;
    while ((i < N))
    {
      if ((inv[(sa[(i + 1)] + 1)] < inv[(sa[i] + 1)]))
      {
        mustInc[i] = true;
        incCount += 1;
      }
      i += 1;
    }
  }
  K -= incCount;
  if (((K <= 0) || (comb[((N + K) - 1)][(K - 1)] < L)))
  {
    puts("Impossible");
    return;
  }
  var fixedAcc = cpp_array((MAX_N + 1));
  memset(fixedAcc, -1, cpp_sizeof((fixedAcc)));
  fixedAcc[N] = (K - 1);
  var states: dynamic;
  states.push_back(State(-1, N, comb[((N + K) - 1)][(K - 1)]));
  var fixed: dynamic;
  fixed.insert(1);
  fixed.insert((-N));
  {
    var cpp_name = 0;
    while ((cpp_name < N))
    {
      var p = (inv[cpp_name] - 1);
      var key = -1;
      var w = 1;
      var prev = (-((*fixed.lower_bound((-p)))));
      var lAcc = if ((prev == -1)) 0 else fixedAcc[prev];
      {
        var i = 0;
        while ((i < int_cpp(states.size())))
        {
          if (((states[i].from_cpp <= p) && (p <= states[i].to)))
          {
            key = i;
          } else
          {
            w = multi(w, states[i].ways);
          }
          i += 1;
        }
      }
      if ((key == -1))
      {
        fixedAcc[p] = lAcc;
        fixed.insert((-p));
        cpp_name += 1;
        continue;
      }
      var from_cpp = states[key].from_cpp;
      var to = states[key].to;
      states.erase((states.begin() + key));
      {
        var a = lAcc;
        while (true)
        {
          var leftWay = 1;
          var rightWay = 1;
          var leftFlex = (a - (if ((from_cpp >= 0)) fixedAcc[from_cpp] else 0));
          var rightFlex = (fixedAcc[to] - a);
          if ((leftFlex > 0))
          {
            leftWay = comb[(((p - from_cpp) - 1) + leftFlex)][leftFlex];
          }
          if ((rightFlex > 0))
          {
            rightWay = comb[(((to - p) - 1) + rightFlex)][rightFlex];
          }
          var newWay = multi(leftWay, rightWay);
          if ((multi(newWay, w) >= L))
          {
            if ((leftWay > 1))
            {
              states.push_back(State(from_cpp, p, leftWay));
            }
            if ((rightWay > 1))
            {
              states.push_back(State(p, to, rightWay));
            }
            fixedAcc[p] = a;
            fixed.insert((-p));
            break;
          } else
          {
            L -= (newWay * w);
          }
          a += 1;
        }
      }
      cpp_name += 1;
    }
  }
  var ans = cpp_array((MAX_N + 1));
  var add = 0;
  {
    var i = 0;
    while ((i < N))
    {
      if (mustInc[i])
      {
        add += 1;
      }
      ans[sa[(i + 1)]] = char(((cpp_char("a") + fixedAcc[i]) + add));
      i += 1;
    }
  }
  puts(ans);
}

func main()
{
  init();
  solve();
  return 0;
}
