// Translated from solution.cpp.

var MAX_N: dynamic = int_cpp((1e5 + 10));

var MAX_K: dynamic = 30;

var INF: dynamic = cpp_int64(2.05e18);

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var sa: dynamic = cpp_array((MAX_N + 1));

var comb: dynamic = cpp_array((MAX_K + 1), ((MAX_N + MAX_K) + 1));

func multi(x: dynamic, y: dynamic) -> dynamic
{
  return  ((((cpp_double(x) * y) < (INF * 1.3)))) ? (x * y) : INF;
}

func init() -> dynamic
{
  scanf("%d%d%lld", (&N), (&K), (&L));
  sa[0] = N;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      scanf("%d", (sa + i));
      sa[i] -= 1;
      i += 1;
    }
  }
}

func prepareCombination() -> dynamic
{
  {
    var n: dynamic = 0;
    while ((n <= (N + K)))
    {
      comb[n][0] = 1;
      if ((n <= K))
      {
        comb[n][n] = 1;
      }
      {
        var k: dynamic = 1;
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
  var from_cpp: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
  var ways: dynamic = cpp_uninitialized();
  func State() -> dynamic
  {
    }
  func State(from_cpp: dynamic, to: dynamic, ways: dynamic) -> dynamic
  {
      self->from_cpp = cpp_construct(from_cpp);
      self->to = cpp_construct(to);
      self->ways = cpp_construct(ways);
    }
}

func solve() -> dynamic
{
  prepareCombination();
  var inv: dynamic = cpp_array((MAX_N + 1));
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      inv[sa[i]] = i;
      i += 1;
    }
  }
  var mustInc: dynamic = cpp_array((MAX_N + 1));
  var incCount: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var fixedAcc: dynamic = cpp_array((MAX_N + 1));
  memset(fixedAcc, -1, cpp_sizeof((fixedAcc)));
  fixedAcc[N] = (K - 1);
  var states: dynamic = cpp_uninitialized();
  states.push_back(State(-1, N, comb[((N + K) - 1)][(K - 1)]));
  var fixed: dynamic = cpp_uninitialized();
  fixed.insert(1);
  fixed.insert((-N));
  {
    var cpp_name: dynamic = 0;
    while ((cpp_name < N))
    {
      var p: dynamic = (inv[cpp_name] - 1);
      var key: dynamic = -1;
      var w: dynamic = 1;
      var prev: dynamic = (-((*fixed.lower_bound((-p)))));
      var lAcc: dynamic =  ((prev == -1)) ? 0 : fixedAcc[prev];
      {
        var i: dynamic = 0;
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
      var from_cpp: dynamic = states[key].from_cpp;
      var to: dynamic = states[key].to;
      states.erase((states.begin() + key));
      {
        var a: dynamic = lAcc;
        while (true)
        {
          var leftWay: dynamic = 1;
          var rightWay: dynamic = 1;
          var leftFlex: dynamic = (a - ( ((from_cpp >= 0)) ? fixedAcc[from_cpp] : 0));
          var rightFlex: dynamic = (fixedAcc[to] - a);
          if ((leftFlex > 0))
          {
            leftWay = comb[(((p - from_cpp) - 1) + leftFlex)][leftFlex];
          }
          if ((rightFlex > 0))
          {
            rightWay = comb[(((to - p) - 1) + rightFlex)][rightFlex];
          }
          var newWay: dynamic = multi(leftWay, rightWay);
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
  var ans: dynamic = cpp_array((MAX_N + 1));
  var add: dynamic = 0;
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  init();
  solve();
  return 0;
}
