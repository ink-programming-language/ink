// Translated from solution.cpp.

func maxHeapify(a: dynamic, n: dynamic, i: dynamic)
{
  var l = (i * 2);
  var r = (l + 1);
  var larg = i;
  if (((l <= n) && (a[l] > a[larg])))
  {
    larg = l;
  }
  if (((r <= n) && (a[r] > a[larg])))
  {
    larg = r;
  }
  if ((larg != i))
  {
    var t = a[larg];
    a[larg] = a[i];
    a[i] = t;
    maxHeapify(a, n, larg);
  }
}

func buildMaxHeap(n: dynamic, a: dynamic)
{
  {
    var i = (n / 2);
    while ((i >= 1))
    {
      maxHeapify(a, n, i);
      i -= 1;
    }
  }
}

func main()
{
  var n: dynamic;
  var i: dynamic;
  read(n);
  var a = cpp_array((n + 1));
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  buildMaxHeap(n, a);
  {
    i = 1;
    while ((i <= n))
    {
      write(" ", a[i]);
      i += 1;
    }
  }
  write("\n");
  return 0;
}
