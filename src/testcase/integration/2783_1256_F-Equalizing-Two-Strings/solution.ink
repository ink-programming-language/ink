// Translated from solution.cpp.

func MAX(m1: dynamic = INT_MIN, m2: dynamic = INT_MIN, m3: dynamic = INT_MIN, m4: dynamic = INT_MIN, m5: dynamic = INT_MIN, m6: dynamic = INT_MIN, m7: dynamic = INT_MIN, m8: dynamic = INT_MIN, m9: dynamic = INT_MIN, m10: dynamic = INT_MIN)
{
  return max(max(max(max(m1, m2), max(m3, m4)), max(m5, m6)), max(max(m7, m8), max(m9, m10)));
}

func MIN(m1: dynamic = INT_MAX, m2: dynamic = INT_MAX, m3: dynamic = INT_MAX, m4: dynamic = INT_MAX, m5: dynamic = INT_MAX, m6: dynamic = INT_MAX, m7: dynamic = INT_MAX, m8: dynamic = INT_MAX, m9: dynamic = INT_MAX, m10: dynamic = INT_MAX)
{
  return min(min(min(min(m1, m2), min(m3, m4)), min(m5, m6)), min(min(m7, m8), min(m9, m10)));
}

func power(x: dynamic, n: dynamic)
{
  var f = 1;
  while (n)
  {
    if ((n & 1))
    {
      f = ((f * x) % 1000000007);
    }
    x = ((x * x) % 1000000007);
    n >>= 1;
  }
  return f;
}

func per(x: dynamic, n: dynamic)
{
  var f = 1;
  while ((n > 0))
  {
    if (((n % 2) == 1))
    {
      f *= x;
    }
    x *= x;
    n /= 2;
  }
  return f;
}

func cin_vector(v: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var num: dynamic;
      read(num);
      v.emplace_back(num);
      i += 1;
    }
  }
}

func cin_deque(d: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var num: dynamic;
      read(num);
      d.emplace_back(num);
      i += 1;
    }
  }
}

func cin_list(l: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var num: dynamic;
      read(num);
      l.emplace_back(num);
      i += 1;
    }
  }
}

func cin_flist(fl: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var num: dynamic;
      read(num);
      fl.emplace_front(num);
      i += 1;
    }
  }
  fl.reverse();
}

func cin_set(st: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var num: dynamic;
      read(num);
      st.emplace(num);
      i += 1;
    }
  }
}

func cin_map(m: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var k: dynamic;
      read(k);
      var s: dynamic;
      read(s);
      m.emplace(k, s);
      i += 1;
    }
  }
}

func cin_2d_vector(v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      {
        var j = 0;
        while ((j < v[i].size()))
        {
          read(v[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func cin_2d_deque(d: dynamic)
{
  {
    var i = 0;
    while ((i < d.size()))
    {
      {
        var j = 0;
        while ((j < d[i].size()))
        {
          read(d[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func cc_set(st: dynamic, r: dynamic, c: dynamic)
{
  {
    var i = 0;
    while ((i < r))
    {
      var row: dynamic;
      {
        var j = 0;
        while ((j < c))
        {
          var n: dynamic;
          read(n);
          row.insert(n);
          j += 1;
        }
      }
      st.insert(row);
      i += 1;
    }
  }
  for (var s in st)
  {
    for (var i in s)
    {
      write(i, " ");
    }
    write("\n");
  }
}

var mod = (1e9 + 7);

var INF = 1e18;

var alphabet = [cpp_char("A"), cpp_char("B"), cpp_char("C"), cpp_char("D"), cpp_char("E"), cpp_char("F"), cpp_char("G"), cpp_char("H"), cpp_char("I"), cpp_char("J"), cpp_char("K"), cpp_char("L"), cpp_char("M"), cpp_char("N"), cpp_char("O"), cpp_char("P"), cpp_char("Q"), cpp_char("R"), cpp_char("S"), cpp_char("T"), cpp_char("U"), cpp_char("V"), cpp_char("W"), cpp_char("X"), cpp_char("Y"), cpp_char("Z")];

var A = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

var a = "abcdefghijklmnopqrstuvwxyz";

var arr = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

var leap = [0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

var PI = 3.1415926535897932384626;

var maxint = INT_MAX;

var maxarr = 1000005;

var btn = 0;

var count = 0;

var cnt = 0;

var sum = 0;

var md = 998244853;

var q = 1e7;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var n: dynamic;
    read(n);
    btn = 0;
    var v: dynamic;
    var v2: dynamic;
    cin_vector(v, n);
    cin_vector(v2, n);
    var it = sv.end();
    sort(sv.begin(), it);
    it = sv2.end();
    sort(sv2.begin(), it);
    {
      var i = 0;
      while ((i < n))
      {
        if ((sv[i] != sv2[i]))
        {
          btn = 1;
          write("NO", "\n");
          break;
        }
        i += 1;
      }
    }
    if ((btn == 1))
    {
      continue;
    }
    sv2.clear();
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if ((sv[i] == sv[(i + 1)]))
        {
          btn = 1;
          write("YES", "\n");
          break;
        }
        i += 1;
      }
    }
    if ((btn == 1))
    {
      continue;
    }
    sv.clear();
    var a = cpp_construct(100);
    var b = cpp_construct(100);
    var ca = 0;
    var cb = 0;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < (v[i] - cpp_char("a"))))
          {
            ca += a[j];
            j += 1;
          }
        }
        a[(v[i] - cpp_char("a"))] += 1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < (v2[i] - cpp_char("a"))))
          {
            cb += b[j];
            j += 1;
          }
        }
        b[(v2[i] - cpp_char("a"))] += 1;
        i += 1;
      }
    }
    if (((ca % 2) == (cb % 2)))
    {
      write("YES", "\n");
      continue;
    }
    write("NO", "\n");
  }
  return 0;
}
