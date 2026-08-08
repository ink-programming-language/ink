// Translated from solution.cpp.

var inf = (1e9 + 7);

var N = (1e5 + 5);

var n: dynamic;

var k: dynamic;

var A = cpp_array(N);

var B = cpp_array(N);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var ans = inf;
  var cnt: dynamic;
  var a = 0;
  var b = 0;
  var l: dynamic;
  var s: dynamic;
  var t: dynamic;
  read(s);
  {
    i = 0;
    while ((i < 6))
    {
      A[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < 3))
    {
      a += A[i];
      i += 1;
    }
  }
  {
    i = 3;
    while ((i < 6))
    {
      b += A[i];
      i += 1;
    }
  }
  if ((a > b))
  {
    k = 1;
    swap(a, b);
  }
  var st = cpp_array(2);
  {
    i = 3;
    while ((i < 6))
    {
      st[(1 - k)].insert(A[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < 3))
    {
      st[k].insert(A[i]);
      i += 1;
    }
  }
  while ((a < b))
  {
    var c: dynamic;
    var d: dynamic;
    c = (*st[0].begin());
    d = (*st[1].rbegin());
    if (((9 - c) > d))
    {
      a += (9 - c);
      st[0].erase(st[0].begin());
    } else
    {
      a += d;
      st[1].erase(st[1].find(d));
    }
    ans += 1;
  }
  write((ans - inf));
}
