// Translated from solution.cpp.

var maxn = (2e5 + 100);

var a = cpp_array(maxn);

var i = 0;

func cons(l: dynamic, r: dynamic)
{
  if ((r < l))
  {
    return;
  }
  if ((a[i] > 0))
  {
    if (((a[i] < l) || (a[i] > r)))
    {
      write(-1, "\n");
      exit(0);
    }
    var x = a[i];
    i += 1;
    cons(l, (x - 1));
    cons((x + 1), r);
  } else
  {
    {
      while ((r >= l))
      {
        a[i] = r;
        r -= 1;
        i += 1;
      }
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  {
    var i = 0;
    while ((i < k))
    {
      read(a[i]);
      i += 1;
    }
  }
  cons(1, n);
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
