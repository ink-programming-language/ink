// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var water: dynamic = cpp_construct(n, 0);
  var land: dynamic = cpp_construct(n, 0);
  var time: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] *= 4;
      time += a[i];
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  read(s);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      water[i] = ( (((s[i] == cpp_char("W")))) ? a[i] : 0);
      land[i] = ( (((s[i] == cpp_char("G")))) ? a[i] : 0);
      if ((i != 0))
      {
        water[i] += water[(i - 1)];
        land[i] += land[(i - 1)];
      }
      i += 1;
    }
  }
  var timePlus: dynamic = 0;
  var usedWater: dynamic = 0;
  var usedLand: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var avaWater: dynamic = (water[i] - usedWater);
      var avaLand: dynamic = (land[i] - usedLand);
      var need: dynamic = (a[i] / 2);
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
