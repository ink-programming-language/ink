// Translated from solution.cpp.

var x: dynamic = cpp_expression("#inclu");

var y: dynamic = cpp_expression("#incl");

var M: dynamic = cpp_expression("#include<");

var a: dynamic = cpp_array(100000);

var st: dynamic = cpp_uninitialized();

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  c *= conj(b);
  if ((c.imag() <= 0))
  {
    return true;
  }
  return false;
}

func check(i: dynamic) -> dynamic
{
  while ((M >= 2))
  {
    var s: dynamic = st[(M - 2)];
    var e: dynamic = st[(M - 1)];
    var S: dynamic = point(a[s].x, a[s].y);
    var E: dynamic = point(a[e].x, a[e].y);
    var N: dynamic = point(a[i].x, a[i].y);
    if (ccw(S, E, N))
    {
      break;
    }
    st.pop_back();
  }
  st.push_back(i);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i].x, a[i].y);
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      check(i);
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 2);
    while ((i >= 0))
    {
      check(i);
      i -= 1;
    }
  }
  st.pop_back();
  write(M, "\n");
  {
    var i: dynamic = M;
    while ((i > 0))
    {
      write(a[st[(i % M)]].x, cpp_char(" "), a[st[(i % M)]].y, "\n");
      i -= 1;
    }
  }
  return 0;
}
