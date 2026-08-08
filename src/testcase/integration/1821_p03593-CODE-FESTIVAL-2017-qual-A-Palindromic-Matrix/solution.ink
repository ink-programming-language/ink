// Translated from solution.cpp.

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  var count: dynamic;
  {
    var i = 0;
    while ((i < h))
    {
      var temp: dynamic;
      read(temp);
      {
        var j = 0;
        while ((j < w))
        {
          count[temp[j]] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var g1 = (((h % 2) * w) % 2);
  var g2 = ((((w % 2) * h) / 2) + (((h % 2) * w) / 2));
  var g4 = ((h / 2) * ((w / 2)));
  while (cpp_update(g1, "--"))
  {
    for (var p in count)
    {
      if ((p.second % 2))
      {
        count[p.first] -= 1;
        break;
      }
    }
  }
  while (cpp_update(g2, "--"))
  {
    for (var p in count)
    {
      if (((p.second % 4) == 2))
      {
        count[p.first] -= 2;
        break;
      }
    }
  }
  for (var p in count)
  {
    if ((p.second % 4))
    {
      write("No", "\n");
      return 0;
    }
  }
  write("Yes", "\n");
}
