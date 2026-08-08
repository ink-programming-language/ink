// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func my_pow(n: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var x = my_pow(n, (p / 2));
  x = ((x * x));
  if ((p & 1))
  {
    x = ((x * n));
  }
  return x;
}

func big_mod(n: dynamic, p: dynamic, m: dynamic)
{
  if ((p == 0))
  {
    return cpp_cast(1);
  }
  var x = big_mod(n, (p / 2), m);
  x = (((x * x)) % m);
  if ((p & 1))
  {
    x = (((x * n)) % m);
  }
  return x;
}

func extract(s: dynamic, ret: dynamic)
{
  (ss >> ret);
  return ret;
}

func itos(n: dynamic)
{
  var s: dynamic;
  while (n)
  {
    s += (((n % 10) + 48));
    n /= 10;
  }
  reverse(s.begin(), s.end());
  return s;
}

func stoi(s: dynamic)
{
  var n = 0;
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
  var x: dynamic;
  var y: dynamic;
  var yy: dynamic;
}

var arr = cpp_array(100005);

func com(a: dynamic, b: dynamic)
{
  return cpp_binary(((a.x > b.x)), "or", (cpp_binary((a.x == b.x), "and", (a.yy < b.yy))));
}

var ar = cpp_array(100001);

var vis = cpp_array(100001);

var a = 0;

var b = 0;

var c = 0;

var r = 0;

var rr = 0;

var res = 0;

var n: dynamic;

var m: dynamic;

var t = 0;

var ks = 0;

var w: dynamic;

var s: dynamic;

var v = cpp_array(100005);

var idx = 0;

var mx = 0;

func dfs(node: dynamic, dis: dynamic)
{
  vis[node] = 1;
  if ((dis > mx))
  {
    mx = dis;
    idx = node;
  }
  {
    var i = 0;
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

func main()
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
      var aa = v[i].size();
      aa -= 1;
      mx += ((((aa) * ((aa + 1)))) / 2);
      i += 1;
    }
  }
  printf("%lld\n", mx);
  return 0;
}
