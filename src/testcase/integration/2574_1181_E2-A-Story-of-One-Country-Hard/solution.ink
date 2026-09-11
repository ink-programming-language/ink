// Translated from solution.cpp.

func read() -> dynamic
{
  var f: dynamic = 1;
  var res: dynamic = 0;
  var ch: dynamic = cpp_uninitialized();
  while (true)
  {
    ch = getchar();
    if ((ch == cpp_char("-")))
    {
      f = (-f);
    }
    if (!((((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
    {
      break;
    }
  }
  while (true)
  {
    res = (((res * 10) + ch) - cpp_char("0"));
    ch = getchar();
    if (!((((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))))
    {
      break;
    }
  }
  return  ((f == 1)) ? res : (-res);
}

func fast_io() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var N: dynamic = 100005;

var M: dynamic = 3010;

var mod: dynamic = (1e9 + 7);

var INF: dynamic = 1e18;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(N);

var B: dynamic = cpp_array(N);

var str: dynamic = cpp_array(N);

var head: dynamic = cpp_array(N);

var to: dynamic = cpp_array((N * 2));

var nxt: dynamic = cpp_array((N * 2));

var tot: dynamic = cpp_uninitialized();

func addEdge(u: dynamic, v: dynamic) -> dynamic
{
  tot += 1;
  nxt[tot] = head[u];
  to[tot] = v;
  head[u] = tot;
}

func mmax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? b : a;
}

func mmin(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

func countOne(set: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while (set)
  {
    res += 1;
    set &= (set - 1);
  }
  return res;
}

func contain(set: dynamic, i: dynamic) -> dynamic
{
  return (((set & ((1 << i)))) > 0);
}

func myPow(a: dynamic, p: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return 1;
  }
  var res: dynamic = myPow(a, (p / 2));
  res *= res;
  res %= mod;
  if (((p % 2) == 1))
  {
    res *= a;
    res %= mod;
  }
  return (res % mod);
}

func addMode(a: dynamic, b: dynamic) -> dynamic
{
  a = (((a + b)) % mod);
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) % mod);
}

func mySwap(a: dynamic, b: dynamic) -> dynamic
{
  var tmp: dynamic = a;
  a = b;
  b = tmp;
}

var p: dynamic = cpp_array(2, 2, N);

func no() -> dynamic
{
  write("NO\n");
  exit(0);
}

func go(sz: dynamic, nums: dynamic) -> dynamic
{
  if ((sz == 1))
  {
    return;
  }
  var mx: dynamic = [(-INF), (-INF), (-INF), (-INF)];
  var its: dynamic = cpp_array(4);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      its[i] = nums[i].begin();
      i += 1;
    }
  }
  var okok: dynamic = false;
  var idxs: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i < sz))
    {
      {
        var j: dynamic = 0;
        while ((j < 4))
        {
          mx[j] = mmax(mx[j], (-p[its[j]->second][(((j >> 1)) ^ 1)][((j & 1))]));
          its[j] += 1;
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < 4))
        {
          if ((mx[j] <= its[j]->first))
          {
            okok = true;
            {
              var it: dynamic = nums[j].begin();
              while ((it != its[j]))
              {
                idxs.push_back(it->second);
                it += 1;
              }
            }
          }
          if (okok)
          {
            break;
          }
          j += 1;
        }
      }
      if (okok)
      {
        break;
      }
      i += 1;
    }
  }
  if ((!okok))
  {
    no();
  }
  var nnums: dynamic = cpp_construct(4);
  for (var i: dynamic in idxs)
  {
    nnums[0].insert(make_pair(p[i][0][0], i));
    nnums[1].insert(make_pair(p[i][0][1], i));
    nnums[2].insert(make_pair(p[i][1][0], i));
    nnums[3].insert(make_pair(p[i][1][1], i));
    nums[0].erase(make_pair(p[i][0][0], i));
    nums[1].erase(make_pair(p[i][0][1], i));
    nums[2].erase(make_pair(p[i][1][0], i));
    nums[3].erase(make_pair(p[i][1][1], i));
  }
  go(idxs.size(), nnums);
  go((sz - idxs.size()), nums);
}

func main() -> dynamic
{
  fast_io();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(p[i][0][0], p[i][0][1]);
      read(p[i][1][0], p[i][1][1]);
      i += 1;
    }
  }
  var nums: dynamic = cpp_construct(4);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      p[i][1][0] *= -1;
      p[i][1][1] *= -1;
      nums[0].insert(make_pair(p[i][0][0], i));
      nums[1].insert(make_pair(p[i][0][1], i));
      nums[2].insert(make_pair(p[i][1][0], i));
      nums[3].insert(make_pair(p[i][1][1], i));
      i += 1;
    }
  }
  go(n, nums);
  write("YES\n");
  return 0;
}
