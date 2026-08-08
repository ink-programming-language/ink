// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a: dynamic;
  var count: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a);
      count.push_back(a);
      i += 1;
    }
  }
  sort(count.begin(), count.end());
  var total = 0;
  var previous = (count[(n - 1)] + 1);
  {
    var i = ((n - 1));
    while ((i >= 0))
    {
      if ((previous <= count[i]))
      {
        previous -= 1;
      } else
      {
        previous = count[i];
      }
      total += (((previous > 0)) * previous);
      i -= 1;
    }
  }
  write(total, "\n");
}
