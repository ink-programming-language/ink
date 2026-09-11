// Translated from solution.cpp.

var INF: dynamic = cpp_expression("#include<i");

var A: dynamic = cpp_array(100000);

func Update(i: dynamic, x: dynamic) -> dynamic
{
  A[i] = x;
}

func Find(x: dynamic, y: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var min: dynamic = INF;
  if ((x < y))
  {
    s = x;
    t = y;
  } else
  {
    s = y;
    t = x;
  }
  {
    var i: dynamic = s;
    while ((i <= t))
    {
      if ((min > A[i]))
      {
        min = A[i];
      }
      i += 1;
    }
  }
  write(min, "\n");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var com: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(n, q);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      A[i] = INF;
      i += 1;
    }
  }
  {
    var j: dynamic = 0;
    while ((j < q))
    {
      read(com, x, y);
      if ((com == 0))
      {
        Update(x, y);
      } else if ((com == 1))
      {
        Find(x, y);
      }
      j += 1;
    }
  }
}
