/**
 * @file test_research.cpp
 * @brief Unit tests for research, restoration and library workflows.
 */

#include <gtest/gtest.h>

#include "museum/artifact.hpp"
#include "museum/book.hpp"
#include "museum/condition_report.hpp"
#include "museum/exceptions.hpp"
#include "museum/library.hpp"
#include "museum/publication.hpp"
#include "museum/research_project.hpp"
#include "museum/researcher.hpp"
#include "museum/restoration_lab.hpp"
#include "museum/restoration_material.hpp"
#include "museum/restoration_task.hpp"

namespace museum {

TEST(BookTest, BorrowAndReturn) {
    Book book("Museum History", "A. Author", "978-1");
    EXPECT_TRUE(book.IsAvailable());
    EXPECT_TRUE(book.Borrow());
    EXPECT_FALSE(book.IsAvailable());
    EXPECT_FALSE(book.Borrow());
    EXPECT_TRUE(book.Return());
    EXPECT_TRUE(book.IsAvailable());
    EXPECT_FALSE(book.Return());
    EXPECT_EQ(book.Title(), "Museum History");
    EXPECT_EQ(book.Author(), "A. Author");
    EXPECT_EQ(book.Isbn(), "978-1");
}

TEST(BookTest, RejectsInvalidData) {
    EXPECT_THROW(Book("", "Author", "isbn"), LibraryException);
    EXPECT_THROW(Book("Title", "", "isbn"), LibraryException);
    EXPECT_THROW(Book("Title", "Author", ""), LibraryException);
}

TEST(LibraryTest, LendsAndReturnsBooks) {
    Library library(2);
    EXPECT_TRUE(library.AddBook(Book("Book A", "Author", "isbn-a")));
    EXPECT_FALSE(library.AddBook(Book("Book A", "Author", "isbn-a")));
    EXPECT_TRUE(library.AddBook(Book("Book B", "Author", "isbn-b")));
    EXPECT_FALSE(library.AddBook(Book("Book C", "Author", "isbn-c")));
    EXPECT_EQ(library.BookCount(), 2U);
    EXPECT_EQ(library.AvailableCount(), 2U);
    EXPECT_TRUE(library.LendBook("isbn-a"));
    EXPECT_FALSE(library.LendBook("isbn-a"));
    EXPECT_EQ(library.AvailableCount(), 1U);
    EXPECT_TRUE(library.ReturnBook("isbn-a"));
    EXPECT_FALSE(library.ReturnBook("isbn-a"));
    EXPECT_THROW(library.LendBook("unknown"), LibraryException);
    EXPECT_THROW(library.ReturnBook("unknown"), LibraryException);
}

TEST(LibraryTest, RejectsInvalidCapacity) {
    EXPECT_THROW(Library(0), LibraryException);
    EXPECT_THROW(Library(-1), LibraryException);
}

TEST(PublicationTest, TracksCitationsAndRecency) {
    Publication publication("Triassic Flora", "r1", 2020);
    EXPECT_TRUE(publication.IsRecent(2025));
    EXPECT_FALSE(publication.IsRecent(2026));
    publication.Cite();
    publication.Cite();
    EXPECT_EQ(publication.Citations(), 2);
    EXPECT_DOUBLE_EQ(publication.ImpactScore(2025), 0.4);
    EXPECT_DOUBLE_EQ(publication.ImpactScore(2020), 2.0);
    EXPECT_EQ(publication.Title(), "Triassic Flora");
    EXPECT_EQ(publication.AuthorId(), "r1");
    EXPECT_EQ(publication.Year(), 2020);
    Researcher author("r1", "Researcher", 45, 1000.0, 2015, "dep", "paleontology");
    Researcher other("r2", "Other", 45, 1000.0, 2015, "dep", "paleontology");
    EXPECT_TRUE(publication.AuthoredBy(author));
    EXPECT_FALSE(publication.AuthoredBy(other));
}

TEST(PublicationTest, RejectsInvalidData) {
    EXPECT_THROW(Publication("", "r1", 2020), ResearchException);
    EXPECT_THROW(Publication("Title", "", 2020), ResearchException);
    EXPECT_THROW(Publication("Title", "r1", -1), ResearchException);
}

TEST(ConditionReportTest, ClassifiesCondition) {
    ConditionReport critical("ex1", 39, "cracks", 5);
    ConditionReport monitored("ex2", 40, "fading", 5);
    ConditionReport stable("ex3", 70, "fine", 5);
    EXPECT_TRUE(critical.IsCritical());
    EXPECT_EQ(critical.Recommendation(), "immediate restoration");
    EXPECT_FALSE(monitored.IsCritical());
    EXPECT_EQ(monitored.Recommendation(), "monitor condition");
    EXPECT_EQ(stable.Recommendation(), "stable");
    EXPECT_EQ(critical.ExhibitId(), "ex1");
    EXPECT_EQ(critical.Score(), 39);
    EXPECT_EQ(critical.Notes(), "cracks");
    EXPECT_EQ(critical.Day(), 5);
    Artifact matching("ex1", "Bowl", "Ancient", 50, 100.0, "bronze", 1.0, "Greece");
    Artifact different("ex2", "Bowl", "Ancient", 50, 100.0, "bronze", 1.0, "Greece");
    EXPECT_TRUE(critical.Concerns(matching));
    EXPECT_FALSE(critical.Concerns(different));
}

TEST(ConditionReportTest, RejectsInvalidData) {
    EXPECT_THROW(ConditionReport("", 50, "notes", 1), InvalidExhibitException);
    EXPECT_THROW(ConditionReport("ex", 101, "notes", 1), InvalidConditionException);
    EXPECT_THROW(ConditionReport("ex", -1, "notes", 1), InvalidConditionException);
    EXPECT_THROW(ConditionReport("ex", 50, "notes", -1), InvalidExhibitException);
}

TEST(ResearchProjectTest, RecordsFindingsAndPublications) {
    ResearchProject project("Bronze Age trade", "r1", 5000.0);
    EXPECT_TRUE(project.IsFunded());
    project.AddFinding("Trade routes crossed the sea");
    project.AddPublication(Publication("Bronze routes", "r1", 2024));
    EXPECT_EQ(project.FindingCount(), 1U);
    EXPECT_EQ(project.PublicationCount(), 1U);
    project.Complete();
    EXPECT_TRUE(project.IsComplete());
    EXPECT_THROW(project.AddFinding("later"), ResearchException);
    EXPECT_THROW(project.AddPublication(Publication("Later", "r1", 2025)), ResearchException);
    EXPECT_THROW(project.Complete(), ResearchException);
    EXPECT_EQ(project.Title(), "Bronze Age trade");
    EXPECT_EQ(project.LeadResearcherId(), "r1");
    EXPECT_DOUBLE_EQ(project.Budget(), 5000.0);
    Researcher lead("r1", "Lead", 45, 1000.0, 2015, "dep", "history");
    Researcher other("r2", "Other", 45, 1000.0, 2015, "dep", "history");
    EXPECT_TRUE(project.LedBy(lead));
    EXPECT_FALSE(project.LedBy(other));
}

TEST(ResearchProjectTest, RejectsInvalidData) {
    EXPECT_THROW(ResearchProject("", "r1", 10.0), ResearchException);
    EXPECT_THROW(ResearchProject("Project", "", 10.0), ResearchException);
    EXPECT_THROW(ResearchProject("Project", "r1", -1.0), ResearchException);
    ResearchProject unfunded("Project", "r1", 0.0);
    EXPECT_FALSE(unfunded.IsFunded());
    EXPECT_THROW(unfunded.AddFinding(""), ResearchException);
}

TEST(RestorationTaskTest, RunsWorkflowAndComputesCost) {
    RestorationTask task("t1", "ex1", "r1", 20.0);
    EXPECT_EQ(task.Status(), "pending");
    EXPECT_THROW(task.AddHours(2.0), RestorationException);
    EXPECT_THROW(task.Complete(), RestorationException);
    task.Start();
    EXPECT_EQ(task.Status(), "in progress");
    EXPECT_THROW(task.Start(), RestorationException);
    task.AddHours(3.0);
    task.AddHours(2.0);
    EXPECT_DOUBLE_EQ(task.Cost(), 100.0);
    task.Complete();
    EXPECT_EQ(task.Status(), "completed");
    EXPECT_THROW(task.AddHours(1.0), RestorationException);
    EXPECT_THROW(task.Complete(), RestorationException);
    EXPECT_EQ(task.Id(), "t1");
    EXPECT_EQ(task.ExhibitId(), "ex1");
    EXPECT_EQ(task.RestorerId(), "r1");
    EXPECT_DOUBLE_EQ(task.HoursSpent(), 5.0);
    EXPECT_DOUBLE_EQ(task.HourlyRate(), 20.0);
}

TEST(RestorationTaskTest, RejectsInvalidData) {
    EXPECT_THROW(RestorationTask("", "ex", "r", 10.0), RestorationException);
    EXPECT_THROW(RestorationTask("t", "", "r", 10.0), RestorationException);
    EXPECT_THROW(RestorationTask("t", "ex", "", 10.0), RestorationException);
    EXPECT_THROW(RestorationTask("t", "ex", "r", -1.0), RestorationException);
    RestorationTask task("t", "ex", "r", 10.0);
    task.Start();
    EXPECT_THROW(task.AddHours(0.0), RestorationException);
}

TEST(RestorationMaterialTest, ConsumesAndRestocks) {
    RestorationMaterial material("Linen", 10.0, 3.0);
    EXPECT_DOUBLE_EQ(material.TotalValue(), 30.0);
    material.Consume(4.0);
    EXPECT_DOUBLE_EQ(material.Quantity(), 6.0);
    material.Restock(4.0);
    EXPECT_DOUBLE_EQ(material.Quantity(), 10.0);
    EXPECT_THROW(material.Consume(11.0), RestorationException);
    EXPECT_THROW(material.Consume(0.0), RestorationException);
    EXPECT_THROW(material.Restock(0.0), RestorationException);
    EXPECT_EQ(material.Name(), "Linen");
    EXPECT_DOUBLE_EQ(material.UnitCost(), 3.0);
}

TEST(RestorationMaterialTest, RejectsInvalidData) {
    EXPECT_THROW(RestorationMaterial("", 1.0, 1.0), RestorationException);
    EXPECT_THROW(RestorationMaterial("Linen", -1.0, 1.0), RestorationException);
    EXPECT_THROW(RestorationMaterial("Linen", 1.0, -1.0), RestorationException);
}

TEST(RestorationLabTest, ManagesTasksAndMaterials) {
    RestorationLab lab(2);
    RestorationTask first("t1", "ex1", "r1", 20.0);
    RestorationTask second("t2", "ex2", "r2", 10.0);
    second.Start();
    EXPECT_TRUE(lab.AssignTask(first));
    EXPECT_THROW(lab.AssignTask(first), RestorationException);
    EXPECT_TRUE(lab.AssignTask(second));
    EXPECT_THROW(lab.AssignTask(RestorationTask("t3", "ex3", "r3", 5.0)), RestorationException);
    EXPECT_EQ(lab.TaskCount(), 2U);
    EXPECT_EQ(lab.PendingCount(), 2U);
    EXPECT_THROW(lab.CompleteTask("t1"), RestorationException);
    lab.CompleteTask("t2");
    EXPECT_EQ(lab.PendingCount(), 1U);
    EXPECT_THROW(lab.CompleteTask("unknown"), RestorationException);

    EXPECT_TRUE(lab.AddMaterial(RestorationMaterial("Linen", 10.0, 3.0)));
    EXPECT_FALSE(lab.AddMaterial(RestorationMaterial("Linen", 5.0, 3.0)));
    lab.ConsumeMaterial("Linen", 2.0);
    EXPECT_THROW(lab.ConsumeMaterial("Silk", 1.0), RestorationException);
    EXPECT_THROW(lab.ConsumeMaterial("Linen", 100.0), RestorationException);
}

TEST(RestorationLabTest, SumsTaskCosts) {
    RestorationLab lab(2);
    RestorationTask first("t1", "ex1", "r1", 20.0);
    first.Start();
    first.AddHours(2.0);
    RestorationTask second("t2", "ex2", "r2", 10.0);
    second.Start();
    second.AddHours(1.0);
    lab.AssignTask(first);
    lab.AssignTask(second);
    EXPECT_DOUBLE_EQ(lab.TotalCost(), 50.0);
}

TEST(RestorationLabTest, RejectsInvalidCapacity) {
    EXPECT_THROW(RestorationLab(0), RestorationException);
    EXPECT_THROW(RestorationLab(-1), RestorationException);
}

}  // namespace museum
