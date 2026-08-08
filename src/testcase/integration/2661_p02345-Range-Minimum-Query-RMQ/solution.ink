// Translated from solution.cpp.

var INF = cpp_expression("#include<i");

var A = cpp_array(100000);

func Update(i: dynamic, x: dynamic)
{
  A[i] = x;
}

func Find(x: dynamic, y: dynamic)
{
  var s: dynamic;
  var t: dynamic;
  var min = INF;
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
    var i = s;
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

func main()
{
  var n: dynamic;
  var q: dynamic;
  var com: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(n, q);
  {
    var i = 0;
    while ((i < n))
    {
      A[i] = INF;
      i += 1;
    }
  }
  {
    var j = 0;
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
