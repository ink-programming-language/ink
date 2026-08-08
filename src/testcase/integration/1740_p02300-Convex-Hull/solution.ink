// Translated from solution.cpp.

var x = cpp_expression("#inclu");

var y = cpp_expression("#incl");

var M = cpp_expression("#include<");

var a = cpp_array(100000);

var st: dynamic;

func ccw(a: dynamic, b: dynamic, c: dynamic)
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

func check(i: dynamic)
{
  while ((M >= 2))
  {
    var s = st[(M - 2)];
    var e = st[(M - 1)];
    var S = point(a[s].x, a[s].y);
    var E = point(a[e].x, a[e].y);
    var N = point(a[i].x, a[i].y);
    if (ccw(S, E, N))
    {
      break;
    }
    st.pop_back();
  }
  st.push_back(i);
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i].x, a[i].y);
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i = 0;
    while ((i < n))
    {
      check(i);
      i += 1;
    }
  }
  {
    var i = (n - 2);
    while ((i >= 0))
    {
      check(i);
      i -= 1;
    }
  }
  st.pop_back();
  write(M, "\n");
  {
    var i = M;
    while ((i > 0))
    {
      write(a[st[(i % M)]].x, cpp_char(" "), a[st[(i % M)]].y, "\n");
      i -= 1;
    }
  }
  return 0;
}
