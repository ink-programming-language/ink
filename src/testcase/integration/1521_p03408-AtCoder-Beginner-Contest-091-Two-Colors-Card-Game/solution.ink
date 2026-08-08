// Translated from solution.cpp.

func main()
{
  var count: dynamic;
  var r: dynamic;
  var b: dynamic;
  read(b);
  {
    var i = 0;
    while ((i < b))
    {
      var tmp: dynamic;
      read(tmp);
      count[tmp] += 1;
      i += 1;
    }
  }
  read(r);
  {
    var i = 0;
    while ((i < r))
    {
      var tmp: dynamic;
      read(tmp);
      count[tmp] -= 1;
      i += 1;
    }
  }
  var max = 0;
  for (var x in count)
  {
    max = max(max, x.second);
  }
  write(max, "\n");
}
