// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var water = cpp_construct(n, 0);
  var land = cpp_construct(n, 0);
  var time = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] *= 4;
      time += a[i];
      i += 1;
    }
  }
  var s: dynamic;
  read(s);
  {
    var i = 0;
    while ((i < n))
    {
      water[i] = (if (((s[i] == cpp_char("W")))) a[i] else 0);
      land[i] = (if (((s[i] == cpp_char("G")))) a[i] else 0);
      if ((i != 0))
      {
        water[i] += water[(i - 1)];
        land[i] += land[(i - 1)];
      }
      i += 1;
    }
  }
  var timePlus = 0;
  var usedWater = 0;
  var usedLand = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var avaWater = (water[i] - usedWater);
      var avaLand = (land[i] - usedLand);
      var need = (a[i] / 2);
      if ((avaWater >= need))
      {
        usedWater += need;
        timePlus += (need * 2);
      } else
      {
        need -= avaWater;
        usedWater += avaWater;
        timePlus += (avaWater * 2);
        if ((avaLand >= need))
        {
          usedLand += need;
          timePlus += (need * 4);
        } else
        {
          need -= avaLand;
          usedLand += avaLand;
          timePlus += (avaLand * 4);
          if ((water[i] > 0))
          {
            timePlus += (((need * 2)) * 3);
          } else
          {
            timePlus += (((need * 2)) * 5);
          }
        }
      }
      i += 1;
    }
  }
  write((((timePlus + time)) / 4));
  return 0;
}
