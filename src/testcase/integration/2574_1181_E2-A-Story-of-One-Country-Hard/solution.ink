// Translated from solution.cpp.

func read()
{
  var f = 1;
  var res = 0;
  var ch: dynamic;
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
  return if ((f == 1)) res else (-res);
}

func fast_io()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var N = 100005;

var M = 3010;

var mod = (1e9 + 7);

var INF = 1e18;

var n: dynamic;

var m: dynamic;

var A = cpp_array(N);

var B = cpp_array(N);

var str = cpp_array(N);

var head = cpp_array(N);

var to = cpp_array((N * 2));

var nxt = cpp_array((N * 2));

var tot: dynamic;

func addEdge(u: dynamic, v: dynamic)
{
  tot += 1;
  nxt[tot] = head[u];
  to[tot] = v;
  head[u] = tot;
}

func mmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) b else a;
}

func mmin(a: dynamic, b: dynamic)
{
  return if ((a < b)) a else b;
}

func countOne(set: dynamic)
{
  var res = 0;
  while (set)
  {
    res += 1;
    set &= (set - 1);
  }
  return res;
}

func contain(set: dynamic, i: dynamic)
{
  return (((set & ((1 << i)))) > 0);
}

func myPow(a: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var res = myPow(a, (p / 2));
  res *= res;
  res %= mod;
  if (((p % 2) == 1))
  {
    res *= a;
    res %= mod;
  }
  return (res % mod);
}

func addMode(a: dynamic, b: dynamic)
{
  a = (((a + b)) % mod);
}

func mul(a: dynamic, b: dynamic)
{
  return ((a * b) % mod);
}

func mySwap(a: dynamic, b: dynamic)
{
  var tmp = a;
  a = b;
  b = tmp;
}

var p = cpp_array(2, 2, N);

func no()
{
  write("NO\n");
  exit(0);
}

func go(sz: dynamic, nums: dynamic)
{
  if ((sz == 1))
  {
    return;
  }
  var mx = [(-INF), (-INF), (-INF), (-INF)];
  var its = cpp_array(4);
  {
    var i = 0;
    while ((i < 4))
    {
      its[i] = nums[i].begin();
      i += 1;
    }
  }
  var okok = false;
  var idxs: dynamic;
  {
    var i = 1;
    while ((i < sz))
    {
      {
        var j = 0;
        while ((j < 4))
        {
          mx[j] = mmax(mx[j], (-p[its[j]->second][(((j >> 1)) ^ 1)][((j & 1))]));
          its[j] += 1;
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < 4))
        {
          if ((mx[j] <= its[j]->first))
          {
            okok = true;
            {
              var it = nums[j].begin();
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
  var nnums = cpp_construct(4);
  for (var i in idxs)
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

func main()
{
  fast_io();
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(p[i][0][0], p[i][0][1]);
      read(p[i][1][0], p[i][1][1]);
      i += 1;
    }
  }
  var nums = cpp_construct(4);
  {
    var i = 0;
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
