// Translated from solution.cpp.

var mod: dynamic = 1000000007;

class Mod
{
  var num: dynamic = cpp_uninitialized();
  func Mod() -> dynamic
  {
      cpp_base_construct(0);
    }
  func Mod(n: dynamic) -> dynamic
  {
      self->num = cpp_construct(((((n % mod) + mod)) % mod));
    }
  func Mod(n: dynamic) -> dynamic
  {
      cpp_base_construct(static_cast(n));
    }
  func cpp_function_1() -> dynamic
  {
      return num;
    }
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return Mod((((a.num + b.num)) % mod));
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) + b);
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return (b + a);
}

func operator(a: dynamic) -> dynamic
{
  return (a + Mod(1));
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return Mod(((((mod + a.num) - b.num)) % mod));
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) - b);
}

func operator(a: dynamic) -> dynamic
{
  return (a - Mod(1));
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return Mod((((cpp_cast(a.num) * b.num)) % mod));
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) * b);
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(b) * a);
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(b) * a);
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a + b));
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a + b));
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a - b));
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a - b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res: dynamic = (((a * a)) ^ ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func mod_pow(a: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res: dynamic = mod_pow(((a * a)), ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func inv(a: dynamic) -> dynamic
{
  return (a ^ ((mod - 2)));
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  assert((b.num != 0));
  return (a * inv(b));
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) / b);
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a / b));
}

var MAX_MOD_N: dynamic = cpp_expression("#includ");

var fact: dynamic = cpp_array(MAX_MOD_N);

var factinv: dynamic = cpp_array(MAX_MOD_N);

func init(amax: dynamic = MAX_MOD_N) -> dynamic
{
  fact[0] = Mod(1);
  factinv[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < (amax - 1)))
    {
      fact[(i + 1)] = (fact[i] * Mod((i + 1)));
      factinv[(i + 1)] = (factinv[i] / Mod((i + 1)));
      i += 1;
    }
  }
}

func comb(a: dynamic, b: dynamic) -> dynamic
{
  return ((fact[a] * factinv[b]) * factinv[(a - b)]);
}

func dfs(rev_edges: dynamic, now: dynamic, used: dynamic) -> dynamic
{
  if (used[now])
  {
    assert(false);
  }
  used[now] = true;
  if ((rev_edges[now].size() == 0))
  {
    return 1;
  }
  assert((rev_edges[now].size() == 1));
  {
    return (1 + dfs(rev_edges, rev_edges[now][0], used));
  }
}

func main() -> dynamic
{
  init();
  while (true)
  {
    var N: dynamic = cpp_uninitialized();
    read(N);
    if ((!N))
    {
      break;
    }
    var edges: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        var a: dynamic = cpp_uninitialized();
        read(a);
        edges.push_back((a - 1));
        rev_edges[(a - 1)].push_back(i);
        i += 1;
      }
    }
    var ok: dynamic = true;
    if (any_of(rev_edges.begin(), rev_edges.end(), __cpp_lambda_2))
    {
      ok = false;
    }
    var ans: dynamic = 0;
    if (ok)
    {
      var loves: dynamic = cpp_construct(N, -1);
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          if ((i == edges[edges[i]]))
          {
            loves[i] = edges[i];
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          if (((rev_edges[i].size() == 2) && (loves[i] == -1)))
          {
            ok = false;
          }
          i += 1;
        }
      }
      if (ok)
      {
        {
          var i: dynamic = 0;
          while ((i < N))
          {
            if ((!used[i]))
            {
              if ((loves[i] != -1))
              {
                used[i] = true;
                if ((rev_edges[i].size() == 1))
                {
                  nums[i] = 1;
                } else
                {
                  for (var e: dynamic in rev_edges[i])
                  {
                    if ((e != edges[i]))
                    {
                      nums[i] = (1 + dfs(rev_edges, e, used));
                    }
                  }
                }
              }
            }
            i += 1;
          }
        }
        if ((find_if(used.begin(), used.end(), __cpp_lambda_3) != used.end()))
        {
          ok = false;
        }
        if (ok)
        {
          ans = 1;
          var v: dynamic = cpp_uninitialized();
          {
            var i: dynamic = 0;
            while ((i < N))
            {
              if ((loves[i] != -1))
              {
                if ((i < edges[i]))
                {
                  i += 1;
                  continue;
                }
                var num: dynamic = (nums[i] + nums[edges[i]]);
                v.push_back(num);
              }
              i += 1;
            }
          }
          var two_num: dynamic = count(v.begin(), v.end(), 2);
          var other_num: dynamic = (v.size() - two_num);
          ans = fact[other_num];
          ans *= mod_pow(2, (two_num + other_num));
          ans *= fact[two_num];
          var kake: dynamic = 0;
          {
            var t: dynamic = 0;
            while ((t <= (two_num / 2)))
            {
              var rest: dynamic = (two_num - (2 * t));
              if ((rest > (other_num + 1)))
              {
                t += 1;
                continue;
              } else
              {
                var plus: dynamic = 0;
                plus = comb((other_num + t), t);
                plus *= comb((other_num + 1), rest);
                plus *= ((other_num + 1) + t);
                kake += plus;
              }
              t += 1;
            }
          }
          ans *= kake;
        }
      }
    }
    write(ans, "\n");
  }
  return 0;
}

func __cpp_lambda_2(v: dynamic) -> dynamic
{
  return (v.size() >= 3);
}

func __cpp_lambda_3(a: dynamic) -> dynamic
{
  return (a == false);
}
