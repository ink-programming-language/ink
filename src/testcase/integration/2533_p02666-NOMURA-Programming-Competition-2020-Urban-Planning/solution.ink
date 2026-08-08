// Translated from solution.cpp.

var MOD = cpp_expression("#include<c");

func f(i: dynamic, n: dynamic)
{
  cpp_macro("for(long long i=0;i<(long long)(n);i++)");
}

var N = cpp_expression("#inclu");

var e = cpp_array(N);

var emp = cpp_array(N);

var used = cpp_array(N);

var a: dynamic;

var cnt: dynamic;

var b = cpp_array(N);

var c = cpp_array(N);

var k = cpp_array(N);

var r = cpp_array(N);

func dfs(k: dynamic)
{
  if (used[k])
  {
    return;
  }
  used[k] = true;
  cnt += 1;
  var sz = e[k].size();
  f(i, sz);
  dfs(e[k][i]);
  return;
}

func main()
{
  var n: dynamic;
  var cc: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var s: dynamic;
  var ans: dynamic;
  scanf("%lld", (&n));
  cc = 0;
  k[0] = 1;
  f(i, (N - 1));
  {
    x = (k[i] * ((i + 1)));
    x %= MOD;
    k[(i + 1)] = x;
  }
  r[0] = 1;
  f(i, (N - 1));
  {
    r[(i + 1)] = (((r[i] * ((n - 1)))) % MOD);
  }
  b[0] = 1;
  var sz = a.size();
  ans = 0;
  s = (n - cc);
  s = (((s * r[sz])) % MOD);
  ans = ((((s - ans) + MOD)) % MOD);
  printf("%lld\n", ans);
  return 0;
}

func f(argument_0: dynamic, argument_1: dynamic)
{
    emp[i] = false;
    used[i] = false;
  }

func f(argument_0: dynamic, argument_1: dynamic)
{
    scanf("%lld", (&x));
    x -= 1;
    if ((x >= 0))
    {
      e[x].push_back(i);
      e[i].push_back(x);
    } else
    {
      emp[i] = true;
    }
  }

func f(argument_0: dynamic, argument_1: dynamic)
{
    if (emp[i])
    {
      cnt = 0;
      dfs(i);
      a.push_back(cnt);
    }
  }

func f(argument_0: dynamic, argument_1: dynamic)
{
    if ((!used[i]))
    {
      cc += 1;
      dfs(i);
    }
  }

func f(argument_0: dynamic, argument_1: dynamic)
{
    b[i] = 0;
    c[i] = 0;
  }

func f(argument_0: dynamic, argument_1: dynamic)
{
      s = (((a[i] * b[(j + 1)])) % MOD);
      s = (((s * k[(j + 1)])) % MOD);
      s = (((s * r[((sz - j) - 2)])) % MOD);
      ans = (((ans + s)) % MOD);
    }

func f(argument_0: dynamic, argument_1: dynamic)
{
    s = (((((a[i] - 1)) * r[(sz - 1)])) % MOD);
    ans = (((ans + s)) % MOD);
    c[0] = 1;
    f(j, (i + 1))[(j + 1)] = (((((b[j] * a[i])) + b[(j + 1)])) % MOD);
    f(j, (i + 2))[j] = c[j];
  }
