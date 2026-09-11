// Translated from solution.cpp.

var nums: dynamic = [0];

func main() -> dynamic
{
  var n: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(nums[i]);
      nhz[nums[i]].push_back(i);
      i += 1;
    }
  }
  var index: dynamic = 0;
  {
    var i: dynamic = 0;
    var next: dynamic = 0;
    while ((i < n))
    {
      var it: dynamic = nhz.find(nums[i]);
      var vec: dynamic = it->second;
      var size: dynamic = vec.size();
      var findFlag: dynamic = false;
      {
        var j: dynamic = 0;
        while ((j < size))
        {
          if ((vec[j] <= i))
          {
            j += 1;
            continue;
          }
          var start: dynamic = i;
          var end: dynamic = vec[j];
          var subSize: dynamic = (end - start);
          if ((subSize > (n - end)))
          {
            break;
          }
          var flag: dynamic = true;
          {
            var z: dynamic = 1;
            while ((z < subSize))
            {
              if ((nums[(start + z)] != nums[(end + z)]))
              {
                flag = false;
                break;
              }
              z += 1;
            }
          }
          if ((!flag))
          {
            j += 1;
            continue;
          }
          index = end;
          findFlag = true;
          break;
          j += 1;
        }
      }
      if (findFlag)
      {
        next = index;
      } else
      {
        next += 1;
      }
      i = next;
    }
  }
  var retSize: dynamic = (n - index);
  write(retSize, "\n");
  write(nums[index]);
  {
    var i: dynamic = (index + 1);
    while ((i < n))
    {
      write(" ", nums[i]);
      i += 1;
    }
  }
  return 0;
}
