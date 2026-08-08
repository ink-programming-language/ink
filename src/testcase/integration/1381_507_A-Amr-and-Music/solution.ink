// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array(n);
  var ind: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var count = 0;
  while ((k >= 0))
  {
    var m = 111;
    var i: dynamic;
    var ans: dynamic;
    {
      i = 0;
      while ((i < n))
      {
        if ((a[i] < m))
        {
          m = a[i];
          ans = i;
        }
        i += 1;
      }
    }
    if ((m == 111))
    {
      break;
    }
    k -= m;
    a[ans] = 111;
    if ((k >= 0))
    {
      count += 1;
      ind.push_back(ans);
    }
  }
  write(count, "\n");
  for (var x in ind)
  {
    write((x + 1), " ");
  }
  return 0;
}
