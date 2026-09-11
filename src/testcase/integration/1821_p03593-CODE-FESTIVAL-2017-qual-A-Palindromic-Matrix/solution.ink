// Translated from solution.cpp.

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  read(h, w);
  var count: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      var temp: dynamic = cpp_uninitialized();
      read(temp);
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          count[temp[j]] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var g1: dynamic = (((h % 2) * w) % 2);
  var g2: dynamic = ((((w % 2) * h) / 2) + (((h % 2) * w) / 2));
  var g4: dynamic = ((h / 2) * ((w / 2)));
  while (cpp_update(g1, "--"))
  {
    for (var p: dynamic in count)
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
    for (var p: dynamic in count)
    {
      if (((p.second % 4) == 2))
      {
        count[p.first] -= 2;
        break;
      }
    }
  }
  for (var p: dynamic in count)
  {
    if ((p.second % 4))
    {
      write("No", "\n");
      return 0;
    }
  }
  write("Yes", "\n");
}
