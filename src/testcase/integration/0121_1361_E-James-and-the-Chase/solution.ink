// Translated from solution.cpp.

var gene: dynamic = cpp_construct(233);

func GET_CHAR() -> dynamic
{
  var maxn: dynamic = 131072;
  var buf: dynamic = cpp_array(maxn);
  var p1: dynamic = buf;
  var p2: dynamic = buf;
  return  (((p1 == p2) && (cpp_assign(p2, "=", (p1 == p2))))) ? EOF : (*cpp_update(p1, "++"));
}

func getInt() -> dynamic
{
  var res: dynamic = cpp_construct(0);
  var c: dynamic = getchar();
  while ((c < cpp_char("0")))
  {
    c = getchar();
  }
  while ((c >= cpp_char("0")))
  {
    res = ((res * 10) + ((c - cpp_char("0"))));
    c = getchar();
  }
  return res;
}

func fastpo(x: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = cpp_construct(1);
  while (n)
  {
    if ((n & 1))
    {
      res = ((res * cpp_cast(x)) % mod);
    }
    x = ((x * cpp_cast(x)) % mod);
    n /= 2;
  }
  return res;
}

func itoa(x: dynamic, width: dynamic = 0) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  if ((x == 0))
  {
    res.push_back(cpp_char("0"));
  }
  while (x)
  {
    res.push_back((cpp_char("0") + (x % 10)));
    x /= 10;
  }
  while ((cpp_cast(res.size()) < width))
  {
    res.push_back(cpp_char("0"));
  }
  reverse(res.begin(), res.end());
  return res;
}

class MI
{
  var a: dynamic = cpp_uninitialized();
  func operator_add(b: dynamic) -> dynamic
  {
      var res: dynamic = [(a + b.a)];
      if ((res.a >= mod))
      {
        res.a -= mod;
      }
      return res;
    }
  func operator_subtract(b: dynamic) -> dynamic
  {
      var res: dynamic = [(a - b.a)];
      if ((res.a <= 0))
      {
        res.a += mod;
      }
      return res;
    }
  func operator_multiply(b: dynamic) -> dynamic
  {
      return [((a * b.a) % mod)];
    }
  func operator_divide(b: dynamic) -> dynamic
  {
      return [((a * fastpo(b.a, (mod - 2), mod)) % mod)];
    }
}

var N: dynamic = 100033;

var LOG: dynamic = 20;

var mod: dynamic = (1e9 + 7);

var inf: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

var dep: dynamic = cpp_array(N);

var vst: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var e: dynamic = cpp_array(N);

var ans: dynamic = cpp_array(N);

var anc: dynamic = cpp_array(N);

var o: dynamic = cpp_array(N);

var cur: dynamic = cpp_array(N);

var insert: dynamic = cpp_array(N);

func dfs(v: dynamic) -> dynamic
{
  vst[v] = true;
  insert[v] = true;
  for (var y: dynamic in e[v])
  {
    if ((!vst[y]))
    {
      dep[y] = (dep[v] + 1);
      if ((!dfs(y)))
      {
        return false;
      }
    } else
    {
      if (((insert[y] == false) || (dep[y] > dep[v])))
      {
        return false;
      }
    }
  }
  insert[v] = false;
  return true;
}

var cpp_name: dynamic = 0;

func d1(v: dynamic) -> dynamic
{
  vst[v] = true;
  cur[dep[v]] = v;
  for (var y: dynamic in e[v])
  {
    if ((!vst[y]))
    {
      dep[y] = (dep[v] + 1);
      d1(y);
      if ((s[v].size() < s[y].size()))
      {
        swap(s[v], s[y]);
      }
      for (var tmp: dynamic in s[y])
      {
        s[v].insert(tmp);
      }
      s[y].clear();
    } else
    {
      s[v].insert(make_pair(dep[y], cpp_update(cpp_name, "++")));
    }
  }
  while (((!s[v].empty()) && (s[v].rbegin()->first >= dep[v])))
  {
    s[v].erase((*s[v].rbegin()));
  }
  anc[v] = -1;
  if ((s[v].size() >= 2))
  {
    ans[v] = false;
  } else if ((s[v].size() == 1))
  {
    anc[v] = cur[s[v].begin()->first];
  }
}

func d2(v: dynamic) -> dynamic
{
  vst[v] = true;
  if ((anc[v] != -1))
  {
    ans[v] &= ans[anc[v]];
  }
  for (var y: dynamic in e[v])
  {
    if ((!vst[y]))
    {
      dep[y] = (dep[v] + 1);
      d2(y);
    }
  }
}

func run() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      e[x].push_back(y);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      o[i] = i;
      swap(o[i], o[((gene() % i) + 1)]);
      i += 1;
    }
  }
  var LIM: dynamic = 100;
  {
    var j: dynamic = 1;
    while ((j <= min(LIM, n)))
    {
      var v: dynamic = o[j];
      fill((insert + 1), ((insert + 1) + n), false);
      fill((vst + 1), ((vst + 1) + n), false);
      dep[v] = 1;
      if (dfs(v))
      {
        fill((vst + 1), ((vst + 1) + n), false);
        fill((ans + 1), ((ans + 1) + n), true);
        d1(v);
        fill((vst + 1), ((vst + 1) + n), false);
        d2(v);
        var cnt: dynamic = 0;
        {
          var i: dynamic = 1;
          while ((i <= n))
          {
            cnt += ans[i];
            i += 1;
          }
        }
        {
          var i: dynamic = 1;
          while ((i <= n))
          {
            s[i].clear();
            i += 1;
          }
        }
        if (((cnt * 5) < n))
        {
          printf("-1\n");
          return;
        } else
        {
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              if (ans[i])
              {
                cnt -= 1;
                printf("%d%c", i,  (cnt) ? cpp_char(" ") : cpp_char("\n"));
              }
              i += 1;
            }
          }
          return;
        }
      }
      j += 1;
    }
  }
  printf("-1\n");
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  {
    var qq: dynamic = 1;
    while ((qq <= t))
    {
      run();
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          e[i].clear();
          i += 1;
        }
      }
      qq += 1;
    }
  }
}
