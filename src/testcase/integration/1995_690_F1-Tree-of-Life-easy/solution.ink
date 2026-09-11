// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func my_pow(n: dynamic, p: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return 1;
  }
  var x: dynamic = my_pow(n, (p / 2));
  x = ((x * x));
  if ((p & 1))
  {
    x = ((x * n));
  }
  return x;
}

func big_mod(n: dynamic, p: dynamic, m: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return cpp_cast(1);
  }
  var x: dynamic = big_mod(n, (p / 2), m);
  x = (((x * x)) % m);
  if ((p & 1))
  {
    x = (((x * n)) % m);
  }
  return x;
}

func extract(s: dynamic, ret: dynamic) -> dynamic
{
  (ss >> ret);
  return ret;
}

func itos(n: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  while (n)
  {
    s += (((n % 10) + 48));
    n /= 10;
  }
  reverse(s.begin(), s.end());
  return s;
}

func stoi(s: dynamic) -> dynamic
{
  var n: dynamic = 0;
  {
    typeof(s.size()) = 0;
    while ((i < (s.size())))
    {
      n = ((n * 10) + ((s[i] - 48)));
      i += 1;
    }
  }
  return n;
}

class info
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var yy: dynamic = cpp_uninitialized();
}

var arr: dynamic = cpp_array(100005);

func com(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_binary(((a.x > b.x)), "or", (cpp_binary((a.x == b.x), "and", (a.yy < b.yy))));
}

var ar: dynamic = cpp_array(100001);

var vis: dynamic = cpp_array(100001);

var a: dynamic = 0;

var b: dynamic = 0;

var c: dynamic = 0;

var r: dynamic = 0;

var rr: dynamic = 0;

var res: dynamic = 0;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var t: dynamic = 0;

var ks: dynamic = 0;

var w: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(100005);

var idx: dynamic = 0;

var mx: dynamic = 0;

func dfs(node: dynamic, dis: dynamic) -> dynamic
{
  vis[node] = 1;
  if ((dis > mx))
  {
    mx = dis;
    idx = node;
  }
  {
    var i: dynamic = 0;
    while ((i < (v[node].size())))
    {
      if ((!vis[v[node][i]]))
      {
        dfs(v[node][i], (dis + 1));
      }
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  read(m);
  m -= 1;
  {
    typeof(m) = 0;
    while ((i < (m)))
    {
      read(a, b);
      v[a].push_back(b);
      v[b].push_back(a);
      i += 1;
    }
  }
  mx = 0;
  {
    typeof(100001) = 1;
    while ((i <= (100001)))
    {
      var aa: dynamic = v[i].size();
      aa -= 1;
      mx += ((((aa) * ((aa + 1)))) / 2);
      i += 1;
    }
  }
  printf("%lld\n", mx);
  return 0;
}
