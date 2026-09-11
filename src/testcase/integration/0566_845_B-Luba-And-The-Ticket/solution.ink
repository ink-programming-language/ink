// Translated from solution.cpp.

var inf: dynamic = (1e9 + 7);

var N: dynamic = (1e5 + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(N);

var B: dynamic = cpp_array(N);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var ans: dynamic = inf;
  var cnt: dynamic = cpp_uninitialized();
  var a: dynamic = 0;
  var b: dynamic = 0;
  var l: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
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
  var st: dynamic = cpp_array(2);
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
    var c: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
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
