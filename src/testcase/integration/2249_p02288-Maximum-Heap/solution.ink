// Translated from solution.cpp.

func maxHeapify(a: dynamic, n: dynamic, i: dynamic) -> dynamic
{
  var l: dynamic = (i * 2);
  var r: dynamic = (l + 1);
  var larg: dynamic = i;
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
    var t: dynamic = a[larg];
    a[larg] = a[i];
    a[i] = t;
    maxHeapify(a, n, larg);
  }
}

func buildMaxHeap(n: dynamic, a: dynamic) -> dynamic
{
  {
    var i: dynamic = (n / 2);
    while ((i >= 1))
    {
      maxHeapify(a, n, i);
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array((n + 1));
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
