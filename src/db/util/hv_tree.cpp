/* @file  hv_tree.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "hv_tree.h"

namespace open_edi {
namespace db {

// for testing
//int main(int argc, char **argv) {
int test_hv_tree() {
    // test box
    Box a(1, 2, 3, 4);
    Box b(0, 0, 0, 0);
    printf("a = %d, %d, %d, %d\n", a.getLLX(), a.getLLY(), a.getURX(), a.getURY());
    printf("b = %d, %d, %d, %d\n", b.getLLX(), b.getLLY(), b.getURX(), b.getURY());
    //b.setBox(a);
    //b = a;
    a = b;
    printf("a = %d, %d, %d, %d\n", a.getLLX(), a.getLLY(), a.getURX(), a.getURY());
    printf("b = %d, %d, %d, %d\n", b.getLLX(), b.getLLY(), b.getURX(), b.getURY());
    if (a == b) {
        printf("you are right, a is equal b\n");
    }
    // test HVCutNode
    HVCutNode<Box> cut_root;
    cut_root.setDir(kHorizontal); // CutNode's initial dir is from TreeNode
    Box a1(5, 8, 16, 15);
    Box a2(7, 20, 10, 23);
    Box a3(6, 28, 11, 35);
    Box a4(8, 50, 19, 56);
    Box a5(5, 53, 17, 58);
    Box a6(6, 70, 9, 78);
    Box a7(3, 5, 20, 7);
    Box a8(1, 5, 22, 9);
    Box a9(8, 80, 12, 85);
    cut_root.insert(&a1);
    cut_root.insert(&a2);
    cut_root.insert(&a3);
    cut_root.insert(&a4);
    cut_root.insert(&a5);
    cut_root.insert(&a6);
    cut_root.insert(&a7);
    cut_root.insert(&a8);
    cut_root.insert(&a9);
    // traverse();
    HVTree<Box> pg_tree;
    // may find way to set cut direction, e.g. by routing direction, layer, pin/instance etc.
    pg_tree.setThreshold(8);
    // pg_tree.setCutDir(Vertical); // use setDefaultDir() for direction
    Box b1(1, 8, 3, 15);
    Box b2(2, 20, 7, 23);
    Box b3(10, 28, 17, 35);
    Box b4(12, 50, 15, 56);
    Box b5(5, 53, 7, 58);
    Box b6(10, 53, 17, 58);
    pg_tree.insert(&a1);
    pg_tree.insert(&b1);
    pg_tree.insert(&a2);
    pg_tree.insert(&b2);
    pg_tree.insert(&a3);
    pg_tree.insert(&b3);
    pg_tree.insert(&a5);
    pg_tree.insert(&b4);
    pg_tree.insert(&a6);
    pg_tree.insert(&b5);
    pg_tree.insert(&b6);
    //printf("\nTraverse PreOrder\n");
    //pg_tree.traverse(PreOrder);
    //printf("\nTraverse InOrder\n");
    //pg_tree.traverse(InOrder);
    //printf("\nTraverse PostOrder\n");
    //pg_tree.traverse(PostOrder);

    // more insert
    Box b7(1, 70, 2, 71);
    Box b8(10, 70, 12, 71);
    Box b9(1, 38, 2, 40);
    Box b10(10, 47, 12, 49);
    Box b11(15, 58, 17, 62);
    pg_tree.insert(&b7);
    pg_tree.insert(&b8);
    pg_tree.insert(&b9);
    pg_tree.insert(&b10);
    pg_tree.insert(&b11);
    printf("\nTraverse PreOrder\n");
    pg_tree.traverse(kPreOrder);

    // remove
    // test remove 1
    //pg_tree.remove(b11);
    //pg_tree.remove(b10);
    // test remove 2
    //pg_tree.remove(b9);
    //pg_tree.remove(b1);
    //pg_tree.remove(b2);
    //pg_tree.remove(b5);
    //pg_tree.remove(b7);
    // test remove 3
    //pg_tree.remove(a1);
    //pg_tree.remove(a2);
    //pg_tree.remove(a3);
    //pg_tree.remove(a5);
    //pg_tree.remove(a6);
    //printf("\nTraverse PreOrder\n");
    //pg_tree.traverse(PreOrder);

    // test search
    std::vector<Box *> my_search_result;
    Box my_search_box(10, 40, 20, 60);
    /*
     * 5, 53, 17, 58
     * 10, 47, 12, 49
     * 12, 50, 15, 56
     * 10, 53, 17, 58
     * 15, 58, 17, 62
     */
    //Box my_search_box(1, 5, 8, 15);
    /*
     * 5, 8, 16, 15
     * 1, 8, 3, 15
     */
    //Box my_search_box(1, 20, 17, 70);
    /*
     * 6, 28, 11, 35
     * 7, 20, 10, 23
     * 5, 53, 17, 58
     * 6, 70, 9, 78
     * 1, 38, 2, 40
     * 2, 20, 7, 23
     * 5, 53, 7, 58
     * 1, 70, 2, 71
     * 10, 47, 12, 49
     * 10, 28, 17, 35
     * 12, 50, 15, 56
     * 10, 53, 17, 58
     * 10, 70, 12, 71
     * 15, 58, 17, 62
     */
    pg_tree.search(my_search_box, &my_search_result);
    printf("search result:\n");
    for (unsigned int i = 0; i < my_search_result.size(); i++) {
        Box box_i = getObjBox(my_search_result[i]);
        printf("%d, %d, %d, %d\n", box_i.getLLX(), box_i.getLLY(), box_i.getURX(), box_i.getURY());
    }

    HVTree<Box> pg_tree2;
    pg_tree2.setThreshold(3);
    pg_tree2.setCutDir(kVertical); // use setDefaultDir() for direction
    pg_tree2.addObject(&a1);
    pg_tree2.addObject(&b1);
    pg_tree2.addObject(&a2);
    pg_tree2.addObject(&b2);
    pg_tree2.addObject(&a3);
    pg_tree2.addObject(&b3);
    pg_tree2.addObject(&a5);
    pg_tree2.addObject(&b4);
    pg_tree2.addObject(&a6);
    pg_tree2.addObject(&b5);
    pg_tree2.addObject(&b6);
    pg_tree2.addObject(&b7);
    pg_tree2.addObject(&b8);
    pg_tree2.addObject(&b9);
    pg_tree2.addObject(&b10);
    pg_tree2.addObject(&b11);
    pg_tree2.divide();
    printf("\nby divide addObject, Traverse PreOrder\n");
    pg_tree2.traverse(kPreOrder);
    my_search_result.clear();
    pg_tree2.search(my_search_box, &my_search_result);
    printf("by divide addObject, search result:\n");
    for (unsigned int i = 0; i < my_search_result.size(); i++) {
        Box box_i = getObjBox(my_search_result[i]);
        printf("%d, %d, %d, %d\n", box_i.getLLX(), box_i.getLLY(), box_i.getURX(), box_i.getURY());
    }

    HVTree<Box> pg_tree3;
    pg_tree3.setThreshold(5);
    pg_tree3.setCutDir(kVertical); // use setDefaultDir() for direction
    std::vector<Box *> boxes;
    boxes.push_back(&a1);
    boxes.push_back(&b1);
    boxes.push_back(&a2);
    boxes.push_back(&b2);
    boxes.push_back(&a3);
    boxes.push_back(&b3);
    boxes.push_back(&a5);
    boxes.push_back(&b4);
    boxes.push_back(&a6);
    boxes.push_back(&b5);
    boxes.push_back(&b6);
    boxes.push_back(&b7);
    boxes.push_back(&b8);
    boxes.push_back(&b9);
    boxes.push_back(&b10);
    boxes.push_back(&b11);
    pg_tree3.divide(&boxes);
    printf("\nby divide push_back, Traverse PreOrder\n");
    pg_tree3.traverse(kPreOrder);
    my_search_result.clear();
    pg_tree3.search(my_search_box, &my_search_result);
    printf("by divide push_back, search result:\n");
    for (unsigned int i = 0; i < my_search_result.size(); i++) {
        Box box_i = getObjBox(my_search_result[i]);
        printf("%d, %d, %d, %d\n", box_i.getLLX(), box_i.getLLY(), box_i.getURX(), box_i.getURY());
    }

    HVTree<Box> pg_tree4;
    pg_tree4.setThreshold(5);
    pg_tree4.setCutDir(kVertical); // use setDefaultDir() for direction
    pg_tree4.addObject(&a1);
    pg_tree4.addObject(&b1);
    pg_tree4.addObject(&a2);
    pg_tree4.addObject(&b2);
    pg_tree4.addObject(&a3);
    pg_tree4.addObject(&b3);
    pg_tree4.addObject(&a5);
    pg_tree4.addObject(&b4);
    pg_tree4.addObject(&a6);
    pg_tree4.addObject(&b5);
    pg_tree4.addObject(&b6);
    pg_tree4.addObject(&b7);
    pg_tree4.addObject(&b8);
    pg_tree4.addObject(&b9);
    pg_tree4.addObject(&b10);
    pg_tree4.addObject(&b11);
    pg_tree4.run(1/*mapper thread num*/, 3/*worker thread num*/);
    printf("\nmulti-thread divide, Traverse PreOrder\n");
    sleep(1);
    pg_tree4.traverse(kPreOrder);
    my_search_result.clear();
    pg_tree4.search(my_search_box, &my_search_result);
    printf("multi-thread divide, search result:\n");
    for (unsigned int i = 0; i < my_search_result.size(); i++) {
        Box box_i = getObjBox(my_search_result[i]);
        printf("%d, %d, %d, %d\n", box_i.getLLX(), box_i.getLLY(), box_i.getURX(), box_i.getURY());
    }

    // for MT testing
    //FILE *fp = fopen("test_boxes_OA_DB", "r");
    FILE *fp = fopen("write_file", "r");
    if (!fp) {
        printf("Cannot open file\n");
        return 0;
    }
#if 0
    FILE *fp_w = fopen("test_write", "w");
    if (!fp_w) {
        printf("Cannot open file test_write\n");
        return 0;
    }
#endif
    HVTree<Box> pg_tree5;
    //float llx = 0;
    //float lly = 0;
    //float urx = 0;
    //float ury = 0;
    int llx = 0;
    int lly = 0;
    int urx = 0;
    int ury = 0;
    //fscanf(fp, "%f %f %f %f", &llx, &lly, &urx, &ury);
    fscanf(fp, "%d %d %d %d", &llx, &lly, &urx, &ury);
    while (!feof(fp)) {
        //fprintf(fp_w, "%d, %d, %d, %d\n", (int)(llx * unit), (int)(lly * unit), (int)(urx * unit), (int)(ury * unit));
        //fprintf(fp_w, "%d, %d, %d, %d\n", llx, lly, urx, ury);
        //memory leak, just for testing
        //Box *rect = new Box((int)(llx * unit), (int)(lly * unit), (int)(urx * unit), (int)(ury * unit));
        Box *rect = new Box(llx, lly, urx, ury);
        pg_tree5.addObject(rect);
        //fscanf(fp, "%f %f %f %f", &llx, &lly, &urx, &ury);
        fscanf(fp, "%d %d %d %d", &llx, &lly, &urx, &ury);
    }
    fclose(fp);
    //fclose(fp_w);

    rusage usage;
    time_t start_t = 0, end_t = 0;
    start_t = time(NULL);
    //clock_t start_c = 0, end_c = 0;
    //start_c = clock();
    //pg_tree5.divide();
    // testing the performance
    //pg_tree5.run(1, 2);
    //pg_tree5.run(1, 4);
    pg_tree5.run(1, 8);
    //pg_tree5.run(1, 16);
    //pg_tree5.run(1, 32);
    //pg_tree5.run(1, 64);
    end_t = time(NULL);
    printf("time = %ld\n", (end_t - start_t));
    //end_c = clock();
    //double clock_time = (double)(end_c - start_c);
    //printf("clock time = %f:\n", clock_time/CLOCKS_PER_SEC);
    getrusage(RUSAGE_SELF, &usage);
    int ru_maxrss = static_cast<int>(usage.ru_maxrss / 1024); // MB
    printf("memory usage = %d\n", ru_maxrss);
    // for 0.68M rects
    //Box search_rect(963.600 1218.213 964.921 1216.466);
    //Box my_search_rect(957580,1217417,960270,1219772);

    // for 10.9M/50M rects
    Box my_search_rect(376062,1316517,377312,1318156);
    //Box my_search_rect(2500959,2984793,2539602,3023704);
    //Box my_search_rect(384341,621091,6676142,6658679);
    my_search_result.clear();
    pg_tree.search(my_search_rect, &my_search_result);
    printf("search result size = %lu\n", my_search_result.size());
    for (unsigned int i = 0; i < my_search_result.size(); i++) {
        Box rect_i = getObjBox(my_search_result[i]);
        printf("%d, %d, %d, %d\n", rect_i.getLLX(), rect_i.getLLY(), rect_i.getURX(), rect_i.getURY());
    }

    printf("Oh! Yeah!\n");
    return 0;
}

}  // namespace db 
}  // namespace open_edi
